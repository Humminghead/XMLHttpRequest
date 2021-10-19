#ifndef JSFUNCTIONS_H
#define JSFUNCTIONS_H

#include <array>
#include <cmath>
#include <iconv.h>
#include <memory>
#include <string>
#include <vector>

namespace jsobjects {

///\brief Uri dictionary
constexpr static const std::array<char, 65> uriCharset = {
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};
//    "A"  // 0
//    "B"  // 1
//    "C"  // 2
//    "D"  // 3
//    "E"  // 4
//    "F"  // 5
//    "G"  // 6
//    "H"  // 7
//    "I"  // 8
//    "J"  // 9
//    "K"  // 10
//    "L"  // 11
//    "M"  // 12
//    "N"  // 13
//    "O"  // 14
//    "P"  // 15
//    "Q"  // 16
//    "R"  // 17
//    "S"  // 18
//    "T"  // 19
//    "U"  // 20
//    "V"  // 21
//    "W"  // 22
//    "X"  // 23
//    "Y"  // 24
//    "Z"  // 25
//    "a"  // 26
//    "b"  // 27
//    "c"  // 28
//    "d"  // 29
//    "e"  // 30
//    "f"  // 31
//    "g"  // 32
//    "h"  // 33
//    "i"  // 34
//    "j"  // 35
//    "k"  // 36
//    "l"  // 37
//    "m"  // 38
//    "n"  // 39
//    "o"  // 40
//    "p"  // 41
//    "q"  // 42
//    "r"  // 43
//    "s"  // 44
//    "t"  // 45
//    "u"  // 46
//    "v"  // 47
//    "w"  // 48
//    "x"  // 49
//    "y"  // 50
//    "z"  // 52
//    "0"  // 52
//    "1"  // 53
//    "2"  // 54
//    "3"  // 55
//    "4"  // 56
//    "5"  // 57
//    "6"  // 58
//    "7"  // 59
//    "8"  // 60
//    "9"  // 61
//    "+"  // 63
//    "/"  // 64
/*!
 * \brief If at out of vector range function return default type of value
 */
template <typename T>
auto getVectorValue(const std::vector<T> &array, size_t at) {
  if (at < array.size())
    return array.at(at);
  return T{};
}
/*!
 * \brief Converts hex value to char
 * \param c
 * \param hex1
 * \param hex2
 */
static inline void hexToChar(unsigned char c, unsigned char &hex1,
                             unsigned char &hex2) {
  hex1 = c / 16;
  hex2 = c % 16;
  hex1 += hex1 <= 9 ? '0' : 'a' - 10;
  hex2 += hex2 <= 9 ? '0' : 'a' - 10;
}
/*!
 * \brief Converts string value to hex value
 * \param String (std::string)
 * \return
 */
static inline auto stringToHex(const std::string &in) {
  return static_cast<int8_t>(std::stoi(in, nullptr, 16));
}
/*!
 * \brief The encodeURIComponent() function encodes a URI component.
 *
 * This function encodes special characters. In addition, it encodes the
 * following characters: , / ? : @ & = + $ #
 *
 * \param Request without URI
 * \return URI encoded string
 */
static std::string encodeURIComponent(std::string &&s) {
  const char *str = s.c_str();
  std::vector<char> v(s.size());
  v.clear();
  for (size_t i = 0, l = s.size(); i < l; i++) {
    char c = str[i];
    if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') || c == '-' || c == '_' || c == '.' ||
        c == '!' || c == '~' || c == '*' || c == '\'' || c == '(' || c == ')') {
      v.push_back(c);
    } else if (c == ' ') {
      v.push_back('+');
    } else {
      v.push_back('%');
      unsigned char d1, d2;
      hexToChar(static_cast<uint8_t>(c), d1, d2);
      v.push_back(static_cast<int8_t>(d1));
      v.push_back(static_cast<int8_t>(d2));
    }
  }
  return std::string(v.cbegin(), v.cend());
}
/*!
 * \brief decodeURIComponent
 * \param s
 * \return
 */
static auto decodeURIComponent(std::string &&s) {
  std::string o;
  for (auto c = s.begin(); c != s.end(); c++) {
    if (*c == '%') {
      o.push_back(stringToHex(std::string((c + 1).base(), 2)));
      c += 2;
    } else if (*c == '+') {
      /// \todo Need check plus action
      o.push_back(' ');
    } else {
      o.push_back(*c);
    }
  }
  return o;
}

/*!
 * \brief Encode from byte array to base64 string
 * \param array of bytes
 */
static auto encodeFromArray(std::vector<uint8_t> &&array) {
  using ArraySizeType = decltype(array.size());
  std::string out{""};
  for (ArraySizeType n = 0; n < static_cast<decltype(n)>(array.size());
       n += 3) {
    out += static_cast<char>(
        uriCharset[static_cast<size_t>(getVectorValue(array, n) >> 2)]);
    out += static_cast<char>(
        uriCharset[static_cast<size_t>(((3 & getVectorValue(array, n)) << 4) |
                                       (getVectorValue(array, n + 1) >> 4))]);
    out += static_cast<char>(uriCharset[static_cast<size_t>(
        ((15 & getVectorValue(array, n + 1)) << 2) |
        (getVectorValue(array, n + 2) >> 6))]);
    out += static_cast<char>(
        uriCharset[static_cast<size_t>(63 & getVectorValue(array, n + 2))]);
  }

  /// \todo Maybe 6 may change
  if (array.size() % 3 == 2) {
    out = out.substr(0, out.length() - 1) + "=";
  } else if (array.size() % 3 == 1) {
    out = out.substr(0, out.length() - 2) + "==";
  }
  return out;
}
/*!
 * \brief Encode URI from hash value
 * \param hash value
 */
static auto encode(double hash) {
  // Calc random numbers from
  std::vector<uint8_t> randomNumbers;
  //  randomNumbers.resize(6);
  randomNumbers.assign(6, 0);
  /// \todo Maybe 6 may change
  for (unsigned long n = 0; n < randomNumbers.size(); n++) {
    auto rn = static_cast<long>(floor(hash / pow(256, 5 - n)));
    randomNumbers[static_cast<size_t>(n)] = 255 & rn;
  }
  return encodeFromArray(std::move(randomNumbers));
}
/*!
 * \brief Decodes URI from Base64 server URI string
 * \param uri - uri sring from server (ex. "CggIlQEQtQEYCA==")
 */
static auto decode(std::string uri) {
  // For check:
  // uri = "CggIlQEQtQEYCA=="
  // d   = std::vector<uint8_t>(10)[10, 8, 8, 149, 1, 16, 181, 1, 24, 8]
#ifdef __WINDOWS
#else
  // Generate UTF-16 values with iconv linux library
  auto desc = iconv_open("UTF-16", "UTF-8");
  if (!desc) {
    return std::unique_ptr<std::vector<uint8_t>>();
  }
  // Result UTF-16 array
  std::vector<uint16_t> utf16;
  utf16.reserve(65);
  utf16.assign(65, 0);
  // iconv
  char *uriData = const_cast<char *>(uriCharset.data());
  size_t uriSize = uriCharset.size(); // Its a count of symbols
  char *utf16Data = reinterpret_cast<char *>(utf16.data());
  size_t utf16Size = uriSize * 2; // Because iconv use char arrays
  iconv(desc, &uriData, &uriSize, &utf16Data, &utf16Size);
  utf16.erase(utf16.begin()); // iconv bug
#endif
  // Decode array
  std::array<uint16_t, 256> n;
  n.fill(0);

  for (size_t i = 0; i < utf16.size(); i++)
    n[utf16[i]] = static_cast<uint16_t>(i);

  uint16_t i{0}, r{0}, o{0}, a{0};
  double s = 0.75 * uri.length();
  size_t c = uri.length();

  auto d = std::make_unique<std::vector<uint8_t>>();
  d->reserve(static_cast<size_t>(s));
  d->assign(static_cast<size_t>(s), 0);

  for (size_t t = 0, l = 0; t < c; t += 4) {
    i = n.at(static_cast<size_t>(uri.at(t)));
    r = n.at(static_cast<size_t>(uri.at(t + 1)));
    o = n.at(static_cast<size_t>(uri.at(t + 2)));
    a = n.at(static_cast<size_t>(uri.at(t + 3)));
    d->at(l++) = static_cast<uint8_t>(i << 2 | r >> 4);
    d->at(l++) = static_cast<uint8_t>((15 & r) << 4 | o >> 2);
    d->at(l++) = static_cast<uint8_t>((3 & o) << 6 | (63 & a));
  }
  //  "=" === e[e.length() - 1] && (s--, "=" === e[e.length() - 2] && s--);
  if (uri[uri.length() - 1] == char{'='} &&
      uri[uri.length() - 2] == char{'='}) {
    d->erase(d->end() - 2, d->end());
  }
  return d;
}

} // namespace jsobjects
#endif // JSFUNCTIONS_H
