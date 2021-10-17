#pragma once
#include "http-server/pch.hpp"
#include "http-server/tcp-client.hpp"
#include "http-server/db/tarantool/enums.hpp"
#include "http-server/db/tarantool/types.hpp"
#include "http-server/log.hpp"

namespace http::db::tarantool {
  class Client : public ITcpClientUser {
    enum class State {
      PreHello,
      Waiting,
      Ready,
    } state_ = State::PreHello;

  public:
    ~Client() override {
      g_log->info("~tarantool::Client");
    }

    void on_data(char* data, size_t size) override {
      std::string_view incoming{ data, size };

      switch (state_) {
        case State::PreHello: {
          http::g_log->debug("incoming.size(): {}, sizeof(HelloPacket): {}", incoming.size(), sizeof(HelloPacket));
          assert(incoming.size() == sizeof(HelloPacket));
          auto hello = reinterpret_cast<HelloPacket*>(data);
          http::g_log->debug("recieved hello from tarantool instance:\n{}", hello->to_string());
          state_ = State::Ready;
          [[fallthrough]];
        }

        case State::Ready: {
          write_some_data();
          break;
        }

        case State::Waiting: {
          http::g_log->info("read_callback on waiting! {}", incoming);

          ServerResponse response;
          response.parse(incoming);
          g_log->info("{}", response.to_string());

          http::g_log->info("waiting done -> ready");
          state_ = State::Ready;

          // write_some_data();
          break;
        }

        default: break;
      }
    }

    void write_some_data() {
      // RequestCall request{ 0, "example_insert" };
      // request.packer().pack_array(2);
      // request.packer().pack_unsigned_int(5);
      // request.packer().pack_array(2);
      // request.packer().pack_str(2);
      // request.packer().pack_str_body("ab", 2);
      // request.packer().pack_double(3.15);


      RequestCall request{ 0, "example_reverse" };
      pack_variadic_tuple(request.packer(), "hello", nullptr, 4.44);
      request.write(client_);

      state_ = State::Waiting;
    }

    void on_hello() override {}
  };
} // namespace http::db::tarantool