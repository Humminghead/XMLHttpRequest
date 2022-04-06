# XMLHttpRequest

XMLHttpRequest it's is a C++ library what are used to interact with servers. The library is very similar to XMLHttpRequest in AJAX and repeats its functionality as much as possible.

This library support only Hypertext Transfer Protocol Version 2 ([HTTP/2](https://datatracker.ietf.org/doc/html/rfc7540)).

# Examples

#### Create a syncronious GET request:
```C++
  auto method = std::string{"GET"};
  auto url = std::string{"https://github.githubassets.com/images/mona-loading-dark.gif"};

  XMLHttpRequest req(method, url, false);

  req.setRequestHeader("accept","image/avif,image/webp,*/*");
  req.setRequestHeader("accept-encoding", "gzip, deflate, br");
  req.setRequestHeader("accept-language", "en-US,en;q=0.5");
  req.setRequestHeader("cache-control", "no-cache");
  req.setRequestHeader("pragma", "no-cache");
  req.setRequestHeader("host", "github.githubassets.com");
  req.setRequestHeader("Sec-Fetch-Dest", "image");
  req.setRequestHeader("Sec-Fetch-Mode", "no-cors");
  req.setRequestHeader("Sec-Fetch-Site", "cross-site");
  req.setRequestHeader("user-agent", "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:88.0) Gecko/20100101 Firefox/88.0");
  req.timeout(3000);
  req.open();
```

#### Create a syncronious POST request:
```C++
XMLHttpRequest req(method, url, false);

  req.setRequestHeader("accept","*/*");
  req.setRequestHeader("accept-language", "en-US,en;q=0.5");
  req.setRequestHeader("accept-encoding", "gzip, deflate, br");
  req.setRequestHeader("content-length", "0");
  req.setRequestHeader("content-type", "application/x-www-form-urlencoded");
  req.setRequestHeader("host", "httpbin.org");
  req.setRequestHeader("origin","https://httpbin.org/post");
  req.setRequestHeader("refer","https://httpbin.org/post");
  req.setRequestHeader("Sec-Fetch-Dest", "empty");
  req.setRequestHeader("Sec-Fetch-Mode", "cors");
  req.setRequestHeader("Sec-Fetch-Site", "cross-site");
  req.setRequestHeader("user-agent", "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:88.0) Gecko/20100101 Firefox/88.0");
  req.timeout(3000);

  req.setOnReadyCallback([&](auto&& result) {
      auto [httpRq, httpRp] = result;
      spdlog::info("{} Status code: {}", pthread_self(), httpRp->statusCode());
      spdlog::info("{} Host: {} Method: {} Scheme: {} Responce data size: {} ", pthread_self(),
                   httpRq->host(), httpRq->method(), httpRq->scheme(), httpRp->contentLength());

      // Print headers
      spdlog::info("Raw responce headers:\r\n {} ", req.getAllResponseHeaders());      
  });

  req.open();
  req.send();
  ```
    
# `XMLHttpRequest methods description:`

```C++
XMLHttpRequest()
```
>    The constructor initializes an XMLHttpRequest. It must be called before any other method calls.

```C++
XMLHttpRequest(std::string &&method, std::string &&url, bool async = false);
```
>    The constructor initializes an XMLHttpRequest with method, url and it type (synchronous by default) 

```C++
XMLHttpRequest(const std::string_view method, const std::string_view url, bool async = false);
```
>    The constructor initializes an XMLHttpRequest with method, url and it type (synchronous by default)

```C++
abort()
```
>    Aborts the request if it has already been sent.

```C++
getAllResponseHeaders()
```
>    Returns all the response headers, separated by CRLF, as a string, or empty string if no response has been received.

```C++
getResponseHeader(const std::string &header)
```
>    Returns the string containing the text of the specified header, or empty string if either the response has not yet been received or the header doesn't exist in the response.

```C++
bool open()
```
>    Initializes a request.

```C++
void open(std::string &&method, std::string &&url)
```
>    Initializes a request in ASYNC mode. The HTTP request method to use "GET" or "POST".

```C++
void open(std::string &&method, std::string &&url, bool async)
```
>    Initializes a request with async flag. The HTTP request method to use "GET" or "POST".

```C++
void open(std::string method, std::string uri, bool async, const std::string user, const std::string password)
```
>    Initializes a request with async flag, user and password for "Authorization" request header. The HTTP request method to use "GET" or "POST".

```C++
void overrideMimeType(std::string &&mime)
```
>    Overrides the MIME type returned by the server.

```C++
void send()
```
>    Sends the request. If the request is synchronous (which is the default), this method returns as soon as the request is sent.

```C++
void send(onReadyCallback &&)
```
>    Sends the request. Call callback function invokes when request is recive full answer from server.

```C++
void send(std::string &&body)
```
>    Sends the POST request with data represented as strig.

```C++
void setRequestHeader(const std::string &header, const std::string &value)
```
>    Sets the value of an HTTP request header. You must call `setRequestHeader()` before `send()`.

```C++
void setRequestHeader(std::string &&header, std::string &&value)
```
>    Sets the value of an HTTP request header. You must call `setRequestHeader()` before `send()`.

```C++
void openRequest(std::string method, std::string url, bool async, std::string user, std::string password)
```
>    Same as `open(std::string method, std::string uri, bool async, const std::string user, const std::string password)`

```C++
bool setOnReadyCallback(onReadyCallback &&cb) noexcept
```
>    Sets a callback function that is called when a request is executed

```C++
bool setOnStateChangedCallback(onStateChangeCallback &&cb) noexcept
```
>    Sets a callback function that is called when the request changes its state

```C++
void setOnTimeoutCallback(onTimeoutCallback &&) noexcept
```
>    Sets a callback function that is called when a request timeout is out.

```C++
void timeout(const size_t milliseconds) noexcept
```
>    Is an size_t representing the number of milliseconds a request can take before automatically being terminated.

```C++
size_t timeout() const
```
>    Return the number of milliseconds a request can take before automatically being terminated.

```C++
std::shared_ptr<Responce> responce() const
```
 >    If responce sucessful the response method returns the response's body content as a pointer of class class Responce, what cointains status code of request, headers, content length and data. Otherwise method return nullptr.The success of the operation depending on the value of the request's responseType property.
