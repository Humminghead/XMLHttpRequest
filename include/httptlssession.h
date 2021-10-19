#ifndef SESSION_H
#define SESSION_H

#include "abstractsession.h"

#include <boost/asio/ssl.hpp>
#include <boost/system/error_code.hpp>

namespace network {

class Header;

class HttpTlsSession : public AbstractSession {
public:
  /**
   * @brief Session
   * @param io_service
   * @param host
   * @param service
   */
  HttpTlsSession(boost::asio::io_service &io_service,
                 boost::asio::ssl::context &tlsCtx, const std::string &host,
                 const std::string &service);

  /**
   * @brief submit
   * @param url
   * @param method
   * @param header
   * @param stream_id
   * @param weight
   * @param exclusive
   * @return
   */
  std::tuple<boost::system::error_code, int32_t>
  submit(std::string_view url, std::string_view method, Header &&header,
         int32_t &stream_id, int32_t &weight, const bool exclusive) noexcept;
  /**
   * @brief Submit request by selected url
   * @param Url for request
   * @param Request method
   * @return Error code and stream id
   */
  std::tuple<boost::system::error_code, int32_t>
  submit(Header &&headers, std::string_view url, Method method) noexcept;

  /**
   * @brief Submit request by selected url
   * @param Url for request
   * @param Request method
   * @param User data
   * @return Error code and stream id
   */
  std::tuple<boost::system::error_code, int32_t>
  submit(Header &&headers, std::string_view url, Method method,
         const std::string &body) noexcept;

  /**
   * @brief submit
   * @param headers
   * @param url
   * @param method
   * @param stream_id
   * @param weight
   * @return
   */
  std::tuple<boost::system::error_code, int32_t>
  submit(Header &&headers, std::string_view url, Method method,
         int32_t stream_id, int32_t weight) noexcept;

  Socket &socket() override;

  /**
   * @brief readSocket
   * @param handler
   */
  void readSocket(Handler handler) noexcept override;

  /**
   * @brief writeSocket
   * @param h
   */
  void writeSocket(Handler h) noexcept override;

  /**
   * @brief shutdownSocket
   */
  void shutdownSocket() noexcept override;

  /**
   * @brief startConnect
   * @param endpoint_it
   * @param ec
   */
  void startConnect(EndpointIt endpoint_it,
                    boost::system::error_code &ec) override;

  /**
   * @brief startResolve
   * @return
   */
  bool startResolve();

  /**
   * @brief stringFromEnum
   * @param method
   * @return
   */
  static constexpr std::string_view stringFromEnum(Method method);

  /**
   * @brief methodFromString
   * @param method
   * @return
   */
  static Method methodFromString(const std::string &method);

private:
  /**
   * @brief Default onResponce handler
   */
  //  static void onResponce(const Responce &);

  /**
   * @brief Default onData handler
   * @param Pointer to data
   * @param Data lenght
   */
  //  static void onData(const uint8_t *data, std::size_t len);

  struct Impl;
  std::unique_ptr<Impl, void (*)(Impl *)> d;
};
} // namespace network
#endif // SESSION_H
