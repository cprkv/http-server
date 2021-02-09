#pragma once
#include "core/log.hpp"
#include "core/error.hpp"
#include <sqlite_modern_cpp.h>
#include <uvw/async.h>
#include <uvw/work.h>
#include <continuable/continuable.hpp>
#include <mutex>
#include <list>
#include <utility>
#include <thread>
#include <sstream>

namespace core::db {
  //---------------------------------------------------------------

  struct SqliteSettings {
    std::string           db_name;
    size_t                max_pool_size{ 64 };
    sqlite::sqlite_config config{
      .flags    = sqlite::OpenFlags::READWRITE | sqlite::OpenFlags::CREATE,
      .zVfs     = nullptr,
      .encoding = sqlite::Encoding::ANY,
    };
  };

  //---------------------------------------------------------------

  struct SqlitePool {
  private:
    std::mutex                     lock_{};
    std::vector<sqlite::database*> free_pool_{};
    size_t                         already_taken_{ 0 };
    SqliteSettings                 settings_;

  public:
    explicit SqlitePool(SqliteSettings settings);

    /**
     * @brief returns db connection pointer if succeeded, nullptr otherwise.
     * thread safe.
     * after usage should be released.
     */
    sqlite::database* acquire();

    /** @brief thread safe. */
    void release(sqlite::database* db);
  };

  //---------------------------------------------------------------
  // ёбнешься какая машинерия
  class Sqlite {
    SqlitePool pool_;

    inline static std::string current_thread_id() {
      std::stringstream ss;
      ss << std::this_thread::get_id();
      return ss.str();
    }

    template <typename TUserData>
    struct WorkData {
      using Action       = std::function<void(sqlite::database&, TUserData&)>;
      using Continuation = std::function<void(TUserData&, const Error&)>;

      TUserData         user_data{};
      sqlite::database* db{ nullptr };
      Action            action;
      Continuation      continuation;
      size_t            retry_count{ 0 };
      Error             error{};
    };

    template <typename TUserData>
    auto work(WorkData<TUserData>* work_data) {
      return [this, work_data]() {
        g_log->debug("calling WorkReq on tid: {}", current_thread_id());
        work_data->db = pool_.acquire();
        if (work_data->db) {
          work_data->error = std::move(Error{});
          try {
            work_data->action(*work_data->db, work_data->user_data);
          } catch (std::exception& ex) {
            work_data->error = std::move(Error{ ex });
          }
        } else {
          work_data->error = std::move(Error{ ErrorCode::NoConnectionsInPool });
          g_log->debug("database pool busy");
        }
      };
    }

    template <typename TUserData>
    auto work_callback(WorkData<TUserData>* work_data) {
      return [this, work_data](const auto&, auto& handle) {
        g_log->debug("calling WorkEvent on tid: {}", current_thread_id());
        if (work_data->db) {
          pool_.release(work_data->db);
        }
        if (!work_data->error || work_data->error.code() != ErrorCode::NoConnectionsInPool) {
          work_data->continuation(work_data->user_data, work_data->error);
          delete work_data;
        } else if (work_data->retry_count < 16) {
          g_log->debug("no result from database. retrying...");
          work_data->retry_count++;
          enqueue_task(work_data);
        }
      };
    }

    template <typename TUserData>
    void enqueue_task(WorkData<TUserData>* work_data) {
      auto loop = uvw::Loop::getDefault();
      g_log->debug("calling with context on tid: {}", current_thread_id());
      auto handle = loop->resource<uvw::WorkReq>(work(work_data));
      handle->template on<uvw::WorkEvent>(work_callback(work_data));
      handle->queue();
    }

  public:
    explicit Sqlite(SqliteSettings settings)
        : pool_{ std::move(settings) } {}

    template <typename TUserData>
    void with_context(typename WorkData<TUserData>::Action       action,
                      typename WorkData<TUserData>::Continuation continuation) {
      auto work_data          = new WorkData<TUserData>{};
      work_data->continuation = std::move(continuation);
      work_data->action       = std::move(action);
      enqueue_task(work_data);
    }

    template <typename TUserData>
    void run_with_context(typename WorkData<TUserData>::Action action) {
      return cti::make_continuable<TUserData>([action = std::move(action), this](auto&& promise) {
        auto work_data          = new WorkData<TUserData>{};
        work_data->continuation = [](TUserData&, const Error&) -> void {
          // todo
        };
        work_data->action = std::move(action);
        enqueue_task(work_data);
      });
    }
  };

  //-----------------------------------------------------------------------

  // todo extract PoolWorker interface
  class SqliteOnContinuable {
    SqlitePool pool_;

    inline static std::string current_thread_id() {
      std::stringstream ss;
      ss << std::this_thread::get_id();
      return ss.str();
    }

    template <typename TUserData>
    struct WorkData {
      using Action = std::function<TUserData(sqlite::database&)>;

      TUserData         user_data{};
      sqlite::database* db{ nullptr };
      Error             error{};

      Action                  action;
      cti::promise<TUserData> promise;

      WorkData(Action action, cti::promise<TUserData>)
          : action{ std::move(action) }
          , promise{ std::move(promise) } {}
    };

    template <typename TUserData>
    auto work(WorkData<TUserData>* work_data) {
      return [this, work_data]() {
        g_log->debug("calling WorkReq on tid: {}", current_thread_id());
        work_data->db = pool_.acquire();

        if (!work_data->db) {
          work_data->error = std::move(Error{ ErrorCode::NoConnectionsInPool });
          g_log->debug("database pool busy");
          return;
        }

        try {
          work_data->user_data = work_data->action(*work_data->db);
        } catch (std::exception& ex) {
          work_data->error = std::move(Error{ ex });
        }
      };
    }

    template <typename TUserData>
    auto work_callback(WorkData<TUserData>* work_data) {
      return [this, work_data](const auto&, auto& handle) {
        g_log->debug("calling WorkEvent on tid: {}", current_thread_id());

        if (work_data->db) {
          pool_.release(work_data->db);
        }

        if (work_data->error) {
          work_data->promise.set_exception(std::make_exception_ptr(work_data->error));
        } else {
          work_data->promise.set_value(std::move(work_data->user_data));
        }

        delete work_data;
      };
    }

    template <typename TUserData>
    void enqueue_task(WorkData<TUserData>* work_data) {
      auto loop = uvw::Loop::getDefault();
      g_log->debug("calling with context on tid: {}", current_thread_id());
      auto handle = loop->resource<uvw::WorkReq>(work(work_data));
      handle->template on<uvw::WorkEvent>(work_callback(work_data));
      handle->queue();
    }

  public:
    explicit SqliteOnContinuable(SqliteSettings settings)
        : pool_{ std::move(settings) } {}

    template <typename TUserData>
    auto with_context(typename WorkData<TUserData>::Action action) {
      return cti::make_continuable<TUserData>([action = std::move(action), this](cti::promise<TUserData>&& promise) {
        auto work_data = new WorkData<TUserData>{ action, promise };
        enqueue_task(work_data);
      });
    }
  };

  //---------------------------------------------------------------
} // namespace core::db
