#include "tcp-server.hpp"
#include "log.hpp"
#include <utility>
#include <uvw.hpp>

using namespace gallery;

struct TcpWriter : public ITcpWriter {
  std::shared_ptr<uvw::TCPHandle> handle;

  explicit TcpWriter(std::shared_ptr<uvw::TCPHandle> handle)
      : handle{ std::move(handle) } {}

  void write(std::unique_ptr<char[]> data, size_t size) override {
    g_log->debug("tcp_writer: write {} bytes", size);
    handle->write(std::move(data), size);
  }

  void done() override {
    g_log->debug("tcp_writer: done");
    handle->close();
  }
};

TcpServer::TcpServer(std::unique_ptr<ITcpReaderFactory> client_factory)
    : handle_{ uvw::Loop::getDefault()->resource<uvw::TCPHandle>() }
    , reader_factory_{ std::move(client_factory) } {

  handle_->on<uvw::ErrorEvent>([](const uvw::ErrorEvent& err, uvw::TCPHandle&) {
    g_log->error("tcp_ handle error: {}", err.what()); // TODO: handle it properly (close handle ...)
  });

  handle_->on<uvw::ListenEvent>([this](const uvw::ListenEvent&, uvw::TCPHandle& handle) {
    auto client_handle = handle.loop().resource<uvw::TCPHandle>();
    auto reader        = reader_factory_->create(std::make_unique<TcpWriter>(client_handle));

    client_handle->on<uvw::CloseEvent>([this, reader](const uvw::CloseEvent&, uvw::TCPHandle& handle) {
      g_log->debug("client_handle: close event");
      reader_factory_->destroy(reader);
    });

    client_handle->on<uvw::EndEvent>([](const uvw::EndEvent&, uvw::TCPHandle& client) {
      g_log->debug("client_handle: end event");
      client.close();
    });

    client_handle->on<uvw::DataEvent>([reader](const uvw::DataEvent& data, uvw::TCPHandle& handle) {
      g_log->debug("client_handle: data event");
      if (bool should_continue{ reader->read(data.data.get(), data.length) }; !should_continue) {
        handle.close();
      }
    });

    g_log->debug("client_handle: accept");
    handle.accept(*client_handle);
    client_handle->read();
  });
}

void TcpServer::listen(const char* addr, int port) {
  handle_->bind(addr, port);
  handle_->listen();
}
