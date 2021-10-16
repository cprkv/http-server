#pragma once
#include "http-server/pch.hpp"

namespace http {
  //---------------------------------------------------------------

  struct ITcpClientUser {
  protected:
    friend class TcpClient;
    TcpClient* client_{ nullptr }; // write data here

  public:
    virtual ~ITcpClientUser()                     = default;
    virtual void on_data(char* data, size_t size) = 0; // get data from here
    virtual void on_hello()                       = 0; // this is time to say something to server
  };

  //---------------------------------------------------------------

  class TcpClient {
    std::shared_ptr<uvw::TCPHandle> handle_;
    std::unique_ptr<ITcpClientUser> user_;

  public:
    explicit TcpClient(std::unique_ptr<ITcpClientUser> user);

    void connect(const std::string& ip, unsigned int port);
    void write(char* data, size_t size);
  };

  //---------------------------------------------------------------
} // namespace http
