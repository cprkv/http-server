#include "http-server/tcp-server.hpp"
#include "http-server/log.hpp"

using namespace core;

//---------------------------------------------------------------

struct TcpWriter : public ITcpWriter {
  std::shared_ptr<uvw::TCPHandle> handle;
  std::string                     data_result;

  explicit TcpWriter(std::shared_ptr<uvw::TCPHandle> handle)
      : handle{ std::move(handle) } {}

  ~TcpWriter() override { g_log->debug("~TcpWriter"); }

  void done() override {
    data_result = data.str();

    g_log->debug("tcp_writer: write {} bytes", data_result.size());
    handle->write(const_cast<char*>(data_result.c_str()),
                  static_cast<unsigned int>(data_result.size()));

    // todo: handle->on< ... written >([handle](){ handle->close() });

    g_log->debug("tcp_writer: done");
    handle->close();
  }
};

//---------------------------------------------------------------

TcpServer::TcpServer(std::unique_ptr<ITcpReaderFactory> client_factory)
    : handle_{ uvw::Loop::getDefault()->resource<uvw::TCPHandle>() }
    , reader_factory_{ std::move(client_factory) } {

  handle_->on<uvw::ErrorEvent>([](const uvw::ErrorEvent& err, uvw::TCPHandle&) {
    g_log->error("tcp_ handle error: {}", err.what()); // TODO: handle it properly (close handle ...)
  });

  handle_->on<uvw::ListenEvent>([this](const uvw::ListenEvent&, uvw::TCPHandle& handle) {
    auto client_handle = handle.loop().resource<uvw::TCPHandle>();
    auto reader        = reader_factory_->create(new TcpWriter(client_handle));

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
      reader->read(data.data.get(), data.length);
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

//---------------------------------------------------------------
