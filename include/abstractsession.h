#pragma once

#include "types.h"

#include <array>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <memory>
#include <utility>

struct nghttp2_session;

namespace network {

class Stream;
class HeaderValue;
class Header;
// class Request;
// class Responce;

class NetworkErrorCategory : public boost::system::error_category {
public:
  const char *name() const noexcept;
  std::string message(int ev) const;
};

class AbstractSession : public std::enable_shared_from_this<AbstractSession> {
public:
  using TcpResolver = boost::asio::ip::tcp::resolver;
  using EndpointIt = boost::asio::ip::tcp::resolver::iterator;
  using Socket = boost::asio::ip::tcp::socket;
  using Handler =
      std::function<void(const boost::system::error_code &ec, std::size_t n)>;

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

  virtual void startConnect(EndpointIt endpoint_it,
                            boost::system::error_code &) = 0;
  virtual Socket &socket() = 0;
  virtual void readSocket(Handler h) noexcept = 0;
  virtual void writeSocket(Handler h) noexcept = 0;
  virtual void shutdownSocket() noexcept = 0;

  boost::system::error_code &getError() const;

  void startResolve(const std::string &host, const std::string &service,
                    boost::system::error_code &ec) noexcept;

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

  std::tuple<boost::system::error_code, int32_t>
  submit(std::string_view url,    //
         std::string_view method, //
         const Header &header,    //
         int32_t &stream_id,      //
         int32_t &weight,         //
         const bool exclusive,    //
         const void *dataProvider = nullptr) noexcept;

  void startPing() noexcept;
  size_t cancelPing() noexcept;

  // See namespaceutils::literals
  Array<64 * 1024> &getReadBuffer() noexcept;
  Array<64 * 1024> &getWriteBuffer() noexcept;

  size_t getWriteBufferSize() const noexcept;

  std::shared_ptr<Stream> createPushStream(uint32_t streamId) noexcept;
  bool addStream(std::shared_ptr<Stream> streamPtr) noexcept;
  std::shared_ptr<Stream> findStream(uint32_t streamId) noexcept;
  std::vector<std::shared_ptr<Stream>> findClosedStreams() noexcept;
  void removeStream(uint32_t streamId) noexcept;
  bool streamsEmpty() noexcept;

  nghttp2_session *rawSession() const;

  void goAway() noexcept;
  boost::system::error_code goAway(uint32_t streamId) noexcept;

  void startWaitConnectionTimer() const;
  void startReadTimer() const;

protected:
  int submitPing(const boost::system::error_code &ec) const;
  int submitPing() const;
  bool shouldStop() const;

private:
  struct Impl;
  std::unique_ptr<Impl, void (*)(Impl *)> d;
};

using AbstractSessionPtr = std::shared_ptr<AbstractSession>;
} // namespace network
