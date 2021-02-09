#pragma once
#include "core/log.hpp"
#include <vector>
#include <mutex>

namespace core {
  //---------------------------------------------------------------

  template <typename TResource>
  class Pool {
    std::mutex              lock_{};
    std::vector<TResource*> free_pool_{};
    size_t                  already_taken_{ 0 };
    size_t                  max_pool_size_;

  protected:
    virtual TResource* create_resource() = 0;

  public:
    explicit Pool(size_t max_pool_size = 64)
        : max_pool_size_{ max_pool_size } {
      free_pool_.reserve(max_pool_size_);
    }

    virtual ~Pool() {
      for (auto* p : free_pool_) {
        delete p;
      }
    }

    /**
     * @brief returns resource pointer if succeeded, nullptr otherwise.
     * thread safe.
     * after usage should be released.
     */
    TResource* acquire() {
      std::unique_lock<std::mutex> _{ lock_ };
      TResource*                   resource;

      if (free_pool_.empty()) {
        if (already_taken_ == max_pool_size_) {
          return nullptr;
        }
        resource = create_resource();
      } else {
        resource = free_pool_.back();
        free_pool_.pop_back();
      }

      already_taken_++;
      g_log->debug("pool already_taken_: {}", already_taken_);

      return resource;
    }

    /** @brief thread safe. */
    void release(TResource* resource) {
      std::unique_lock<std::mutex> _{ lock_ };
      free_pool_.push_back(resource);
      already_taken_--;
      g_log->debug("pool already_taken_: {}", already_taken_);
    }
  };

  //---------------------------------------------------------------
} // namespace core