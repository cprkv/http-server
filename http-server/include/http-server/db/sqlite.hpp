#pragma once
#include "http-server/pool-worker.hpp"
#include "http-server/functor.hpp"
#include <sqlite_modern_cpp.h>

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

  class SqlitePool : public Pool<sqlite::database> {
    SqliteSettings settings_;

  protected:
    sqlite::database* create_resource() override {
      return new sqlite::database(settings_.db_name, settings_.config);
    }

  public:
    explicit SqlitePool(SqliteSettings settings)
        : settings_{ std::move(settings) }
        , Pool<sqlite::database>{ settings.max_pool_size } {}

    ~SqlitePool() override = default;
  };

  //-----------------------------------------------------------------------

  class Sqlite {
    PoolWorker<sqlite::database> pool_worker_;

  public:
    explicit Sqlite(SqliteSettings settings)
        : pool_worker_{ std::make_unique<core::db::SqlitePool>(std::move(settings)) } {}

    template <typename TAction>
    auto with_connection(TAction&& action) {
      using TUserData = typename FunctorInfo<TAction>::ReturnType;
      return pool_worker_.template with_resource<TUserData>(action);
    }
  };

  //---------------------------------------------------------------
} // namespace core::db
