#include "http-server/tcp-client.hpp"
#include "http-server/log.hpp"

using namespace http;

//---------------------------------------------------------------

TcpClient::TcpClient(std::unique_ptr<ITcpClientUser> user)
    : handle_{ uvw::Loop::getDefault()->resource<uvw::TCPHandle>() }
    , user_{ std::move(user) } {
  user_->client_ = this;

  handle_->on<uvw::ErrorEvent>([](const uvw::ErrorEvent& err, uvw::TCPHandle&) {
    g_log->error("TcpClient: error event: {}", err.what()); // TODO: handle it properly (close handle ...)
  });

  handle_->on<uvw::ConnectEvent>([this](const uvw::ConnectEvent&, uvw::TCPHandle&) {
    g_log->debug("TcpClient: connect event");

    handle_->on<uvw::CloseEvent>([this](const uvw::CloseEvent&, uvw::TCPHandle&) {
      g_log->debug("TcpClient: close event");
      user_.reset();
    });

    handle_->on<uvw::EndEvent>([this](const uvw::EndEvent&, uvw::TCPHandle&) {
      g_log->debug("TcpClient: end event");
      handle_->close();
    });

    handle_->on<uvw::DataEvent>([this](const uvw::DataEvent& data, uvw::TCPHandle&) {
      g_log->debug("TcpClient: data event");
      user_->on_data(data.data.get(), data.length);
    });

    user_->on_hello();
    handle_->read();
  });
}

void TcpClient::connect(const std::string& ip, unsigned int port) {
  handle_->connect(ip, port);
  // TODO: handle_->keepAlive(true, std::chrono::minutes{ 1 });
}

void TcpClient::write(char* data, size_t size) {
  handle_->write(data, static_cast<unsigned int>(size));
  // TODO: handle->on< ... written >([handle](){ ... });
}

//---------------------------------------------------------------
