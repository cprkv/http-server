#include "core/db/sqlite.hpp"
#include "core/log.hpp"
#include <utility>

using namespace core;
using namespace core::db;

//---------------------------------------------------------------

SqlitePool::SqlitePool(SqliteSettings settings)
    : settings_{ std::move(settings) } {
  free_pool_.reserve(settings_.max_pool_size);
}

sqlite::database* SqlitePool::acquire() {
  std::unique_lock<std::mutex> _{ lock_ };
  sqlite::database*            database;

  if (free_pool_.empty()) {
    if (already_taken_ == settings_.max_pool_size) {
      return nullptr;
    }
    database = new sqlite::database(settings_.db_name, settings_.config);
  } else {
    database = free_pool_.back();
    free_pool_.pop_back();
  }

  already_taken_++;
  g_log->debug("sqlite pool already_taken_: {}", already_taken_);

  return database;
}

void SqlitePool::release(sqlite::database* db) {
  std::unique_lock<std::mutex> _{ lock_ };
  free_pool_.push_back(db);
  already_taken_--;
  g_log->debug("sqlite pool already_taken_: {}", already_taken_);
}

//---------------------------------------------------------------

