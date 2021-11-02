#pragma once

#include "literals.h"
#include "types.h"

#include <array>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/error_category.hpp>
#include <memory>
#include <utility>

/**
 * Forward declations
 **/
struct nghttp2_session;

namespace boost::asio {
class io_context;
typedef io_context io_service;
} // namespace boost::asio

namespace network {
class Stream;
class HeaderValue;
class Header;
} // namespace network

/**
 * Class declation
 **/
namespace network {
using namespace network::literals;

class NetworkErrorCategory : public boost::system::error_category {
public:
  const char *name() const noexcept;
  std::string message(int ev) const;
};

class AbstractSession : public std::enable_shared_from_this<AbstractSession> {
public:
  using ErrorCode = boost::system::error_code;
  using TcpResolver = boost::asio::ip::tcp::resolver;
  using EndpointIt = boost::asio::ip::tcp::resolver::iterator;
  using Socket = boost::asio::ip::tcp::socket;
  using Handler = std::function<void(const ErrorCode &ec, std::size_t n)>;

  using SessionHolder = std::function<std::shared_ptr<AbstractSession>()>;

  template <size_t N> using Array = std::array<uint8_t, N>;

  /**
   * @brief Supported header methods
   */
  enum class Method { Get, Post, Unknown };

  AbstractSession(boost::asio::io_service &io_service,
                  const boost::posix_time::time_duration &connect_timeout,
                  const boost::posix_time::time_duration &read_timeout);

  virtual ~AbstractSession();

  virtual void startConnect(EndpointIt endpoint_it, ErrorCode &) = 0;
  virtual Socket &socket() = 0;
  virtual void readSocket(Handler h) noexcept = 0;
  virtual void writeSocket(Handler h) noexcept = 0;
  virtual void shutdownSocket() noexcept = 0;

  ErrorCode &getError() const;

  void startResolve(const std::string &host, const std::string &service,
                    ErrorCode &ec) noexcept;

  void stop();

  void read() noexcept;
  void write() noexcept;

  bool isStopped() const noexcept;
  void setStopped(bool stopped) noexcept;

  void onSocketConnected(EndpointIt it);

  void setOnReadyCallback(onReadyCallback &&cb);
  onReadyCallback &getOnReadyCallback() const;

  void setOnStopCallback(onStopCallback &&cb);
  onStopCallback &getOnStopCallback() const;

  void setOnConnectCallback(onConnectCallback &&cb);
  onConnectCallback &getOnConnectCallback() const;

  void setOnStateChangeCallback(onStateChangeCallback &&cb);
  onStateChangeCallback &getOnStateChangeCallback() const;

  std::tuple<ErrorCode, int32_t>
  submit(std::string_view url,    //
         std::string_view method, //
         const Header &header,    //
         int32_t &stream_id,      //
         int32_t &weight,         //
         const bool exclusive,    //
         const void *dataProvider = nullptr) noexcept;

  void startPing() noexcept;
  size_t cancelPing() noexcept;

  Array<64_Kb> &getReadBuffer() noexcept;
  Array<64_Kb> &getWriteBuffer() noexcept;

  size_t getWriteBufferSize() const noexcept;

  std::shared_ptr<Stream> createPushStream(uint32_t streamId) noexcept;
  bool addStream(std::shared_ptr<Stream> streamPtr) noexcept;
  std::shared_ptr<Stream> findStream(uint32_t streamId) noexcept;
  std::vector<std::shared_ptr<Stream>> findClosedStreams() noexcept;
  void removeStream(uint32_t streamId) noexcept;
  bool streamsEmpty() noexcept;

  nghttp2_session *rawSession() const;

  void goAway() noexcept;
  ErrorCode goAway(uint32_t streamId) noexcept;

  void startWaitConnectionTimer() const;
  void startReadTimer() const;

  void setMimeOverridenType(std::string &&mime);
  const std::string &getMimeOverridenType() const;

protected:
  int submitPing(const ErrorCode &ec) const;
  int submitPing() const;
  bool shouldStop() const;

private:
  struct Impl;
  std::unique_ptr<Impl, void (*)(Impl *)> d;
};

using AbstractSessionPtr = std::shared_ptr<AbstractSession>;
} // namespace network
