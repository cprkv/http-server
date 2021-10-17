#pragma once
#include "http-server/pch.hpp"
#include "http-server/tcp-client.hpp"
#include "http-server/log.hpp"
#include "http-server/db/tarantool/msgpack-ext.hpp"

namespace http::db::tarantool {
  struct Error {
    std::string  type;
    std::string  file;
    unsigned int line;
    std::string  message;
    unsigned int err_no;
    unsigned int err_code; // IProtoError

    void unpack(const msgpack::object& object) {
      if (object.type != msgpack::type::MAP) {
        throw msgpack::type_error();
      }
      for (size_t i = 0; i < object.via.map.size; i++) {
        auto& kv = object.via.map.ptr[i];
        switch (kv.key.as<unsigned int>()) {
          case 0: type = kv.val.as<decltype(type)>(); break;
          case 1: file = kv.val.as<decltype(file)>(); break;
          case 2: line = kv.val.as<decltype(line)>(); break;
          case 3: message = kv.val.as<decltype(message)>(); break;
          case 4: err_no = kv.val.as<decltype(err_no)>(); break;
          case 5: err_code = kv.val.as<decltype(err_code)>(); break;
        }
      }
    }

    std::string to_string() {
      std::stringstream ss;
      ss << "  type: " << type << "\n";
      ss << "  file: " << file << "\n";
      ss << "  line: " << line << "\n";
      ss << "  message: " << message << "\n";
      ss << "  err_no: " << err_no << "\n";
      ss << "  err_code: " << err_code << "\n";
      return ss.str();
    }
  };

#pragma pack(push, 1)
  struct HelloPacket {
    char greeting_line_1[64];
    char greeting_line_2[64];

    std::string_view get_greeting_line_1() {
      auto line = std::string_view{ greeting_line_1, sizeof(greeting_line_1) };
      return line.substr(0, line.find('\n'));
    }

    std::string_view get_greeting_line_2() {
      auto line = std::string_view{ greeting_line_2, sizeof(greeting_line_2) };
      return line.substr(0, line.find('\n'));
    }

    std::string to_string() {
      std::stringstream ss;
      ss << "  greeting_line_1: " << get_greeting_line_1() << "\n";
      ss << "  greeting_line_2: " << get_greeting_line_2() << "\n";
      return ss.str();
    }
  };
#pragma pack(pop)

  struct Header {
    IProtoType   request_type;
    unsigned int sync;
    unsigned int schema_version;

    IProtoError get_error_code() const { return (IProtoError) (request_type ^ IProtoType_TypeError); }
    bool        is_error() const { return (request_type & IProtoType_TypeError) != 0; }

    void parse(const msgpack::object& object) {
      msgpack::type::assoc_vector<unsigned int, unsigned int> dst;
      object.convert(dst);
      for (const auto& [key, value] : dst) {
        switch (key) {
          case IProtoKey_RequestType: request_type = static_cast<IProtoType>(value); break;
          case IProtoKey_Sync: sync = value; break;
          case IProtoKey_SchemaVersion: schema_version = value; break;
          default: assert(false); break;
        }
      }
    }

    std::string to_string() {
      std::stringstream ss;
      if (is_error()) {
        ss << "  request_type: (error) " << http::db::tarantool::to_string(get_error_code()) << "\n";
      } else {
        ss << "  request_type: " << http::db::tarantool::to_string(request_type) << "\n";
      }
      ss << "  sync: " << sync << "\n";
      ss << "  schema_version: " << schema_version << "\n";
      return ss.str();
    }
  };

  class Body {
    std::vector<Error> errors_;
    std::string        data_;

  public:
    const std::vector<Error>& get_errors() const { return errors_; }

    void parse(const msgpack::object& object) {
      msgpack::type::assoc_vector<unsigned int, msgpack::object> data;
      object.convert(data);

      for (const auto& [key, value] : data) {
        if (key == IProtoKey_Data) {
          set_data(value);
        } else if (key == IProtoKey_Error) {
          set_error(value);
        }
      }
    }

    std::string to_string() {
      std::stringstream ss;
      for (auto& error : errors_) {
        ss << " <body>: (error) " << error.to_string() << "\n";
      }
      if (!data_.empty()) {
        ss << " <body>: " << data_ << "\n";
      }
      return ss.str();
    }

  private:
    void set_data(const msgpack::object& object) {
      std::stringstream ss;
      ss << object;
      data_ = ss.str();
    }

    void set_error(const msgpack::object& object) {
      auto errors = object.as<
          msgpack::type::assoc_vector<
              unsigned int,
              std::vector<msgpack::object>>>();

      for (const auto& [key, value] : errors) {
        if (key == 0) { // stack error
          for (auto& error : value) {
            Error err_typed;
            err_typed.unpack(error);
            errors_.emplace_back(std::move(err_typed));
          }
        }
      }
    }
  };

  struct ServerResponse {
    Header header;
    Body   body;

    void parse(std::string_view data) {
      msgpack::unpacker pac;
      pac.reserve_buffer(data.size());
      memcpy(pac.buffer(), data.data(), data.size());
      pac.buffer_consumed(data.size());

      msgpack::object_handle oh;

      assert(pac.next(oh)); //size, skipping
      auto size = oh.get().as<unsigned int>();
      assert(size + 5 == data.size());

      assert(pac.next(oh));
      header.parse(oh.get());

      assert(pac.next(oh));
      body.parse(oh.get());

      assert(!pac.next(oh));
    }

    std::string to_string() {
      std::stringstream ss;
      ss << "header:\n"
         << header.to_string()
         << "body:\n"
         << body.to_string();
      return ss.str();
    }
  };

  class RequestCall {
    msgpack::sbuffer                  buffer_;
    msgpack::packer<msgpack::sbuffer> packer_{ buffer_ };

  public:
    RequestCall(unsigned int sync, std::string_view func_name) {
      // header
      pack_variadic_map(
          packer_,
          (unsigned int) IProtoKey_RequestType, (unsigned int) IProtoType_Call,
          (unsigned int) IProtoKey_Sync, sync);

      // body
      packer_.pack_map(2);

      pack_variadic(
          packer_,
          (unsigned int) IProtoKey_FunctionName, func_name,
          (unsigned int) IProtoKey_Tuple);
    }

    RequestCall(const RequestCall&) = delete;
    RequestCall& operator=(const RequestCall&) = delete;

    // write arguments here
    msgpack::packer<msgpack::sbuffer>& packer() { return packer_; }

    void write(TcpClient* client) {
      auto size_buffer = msgpack::sbuffer{};
      auto size_packer = msgpack::packer{ size_buffer };
      size_packer.pack_unsigned_int((uint32_t) buffer_.size());

      client->write(const_cast<char*>(size_buffer.data()), size_buffer.size());
      client->write(const_cast<char*>(buffer_.data()), buffer_.size());
    }
  };
} // namespace http::db::tarantool
