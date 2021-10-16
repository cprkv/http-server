#pragma once
#include "http-server/pch.hpp"
#include "http-server/error.hpp"
#include "http-server/pool.hpp"

namespace http {
  //---------------------------------------------------------------

  template <typename TResource>
  class PoolWorker {
    std::unique_ptr<Pool<TResource>> pool_;

    template <typename TUserData>
    struct WorkData {
      using Action = std::function<TUserData(TResource&)>;

      TUserData               user_data{};
      TResource*              resource{ nullptr };
      Error                   error{};
      Action                  action;
      cti::promise<TUserData> promise;

      WorkData(Action action, cti::promise<TUserData> promise)
          : action{ std::move(action) }
          , promise{ std::move(promise) } {}
    };

    template <typename TUserData>
    auto work(WorkData<TUserData>* work_data) {
      return [this, work_data]() {
        work_data->resource = pool_->acquire();

        if (!work_data->resource) {
          work_data->error = std::move(Error{ ErrorCode::NoConnectionsInPool });
          g_log->debug("pool busy");
          return;
        }

        try {
          work_data->user_data = work_data->action(*work_data->resource);
        } catch (std::exception& ex) {
          work_data->error = std::move(Error{ ex });
        }
      };
    }

    template <typename TUserData>
    auto work_callback(WorkData<TUserData>* work_data) {
      return [this, work_data](const auto&, auto& handle) {
        if (work_data->resource) {
          pool_->release(work_data->resource);
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
      auto handle = loop->resource<uvw::WorkReq>(work(work_data));
      handle->template on<uvw::WorkEvent>(work_callback(work_data));
      handle->queue();
    }

  public:
    explicit PoolWorker(std::unique_ptr<Pool<TResource>> pool)
        : pool_{ std::move(pool) } {}

    template <typename TUserData>
    auto with_resource(typename WorkData<TUserData>::Action action) {
      // todo retry on pool busy
      return cti::make_continuable<TUserData>([action = std::move(action), this](cti::promise<TUserData>&& promise) {
        auto work_data = new WorkData<TUserData>(action, std::move(promise));
        enqueue_task(work_data);
      });
    }
  };

  //---------------------------------------------------------------
} // namespace http
