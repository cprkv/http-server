#pragma once
#include "core/log.hpp"
#include <sqlite_modern_cpp.h>
#include <mutex>
#include <list>
#include <utility>
#include <uvw/async.h>
#include <uvw/work.h>
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
      TUserData                                          user_data{};
      sqlite::database*                                  db{ nullptr };
      std::function<void(sqlite::database&, TUserData&)> action;
      std::function<void(TUserData*)>                    continuation;
      size_t                                             retry_count{ 0 };
    };

    template <typename TUserData>
    auto work(WorkData<TUserData>* work_data) {
      return [this, work_data]() {
        g_log->debug("calling WorkReq on tid: {}", current_thread_id());
        work_data->db = pool_.acquire();
        if (work_data->db) {
          work_data->action(*work_data->db, work_data->user_data);
        } else {
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
          work_data->continuation(&work_data->user_data);
          delete work_data;
        } else if (work_data->retry_count < 16) {
          g_log->debug("no result from database. retrying...");
          work_data->retry_count++;
          enqueue_task(work_data); // todo enqueue after some timeout
        } else {
          g_log->debug("no result from database and retry count exceeded.");
          work_data->continuation(nullptr);
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
    void with_context(std::function<void(sqlite::database&, TUserData&)> action,
                      std::function<void(TUserData*)>                    continuation) {
      auto work_data          = new WorkData<TUserData>{};
      work_data->continuation = std::move(continuation);
      work_data->action       = std::move(action);
      enqueue_task(work_data);
    }
  };

  //---------------------------------------------------------------
} // namespace core::db
