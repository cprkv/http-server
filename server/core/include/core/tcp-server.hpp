#pragma once
#include <uvw/tcp.h>
#include <sstream>

namespace core {
  //---------------------------------------------------------------

  struct ITcpWriter {
    std::stringstream data{};

    virtual ~ITcpWriter() = default;
    virtual void done()   = 0;
  };

  //---------------------------------------------------------------

  struct ITcpReader {
    virtual ~ITcpReader()                      = default;
    virtual void read(char* data, size_t size) = 0;
  };

  //---------------------------------------------------------------

  struct ITcpReaderFactory {
    virtual ~ITcpReaderFactory()                                   = default;
    virtual ITcpReader* create(std::unique_ptr<ITcpWriter> writer) = 0;
    virtual void        destroy(ITcpReader* client)                = 0;
  };

  //---------------------------------------------------------------

  class TcpServer {
    std::shared_ptr<uvw::TCPHandle>    handle_;
    std::unique_ptr<ITcpReaderFactory> reader_factory_;

  public:
    explicit TcpServer(std::unique_ptr<ITcpReaderFactory> client_factory);

    void listen(const char* addr, int port);
  };

  //---------------------------------------------------------------
} // namespace core
