#include "httpheader.h"
#include "httpheadervalue.h"

namespace network {

struct Header::Impl {
  Impl(Fields &&f) : fields_{std::move(f)} {}
  Fields fields_;
};

Header::Header() : d(new Impl(Fields{}), [](auto p) { delete p; }) {}

Header::Header(Fields &&list)
    : d{new Impl(std::move(list)), [](Impl *p) { delete p; }} {}

void Header::emplace(std::string &&name, HeaderValue &&value) noexcept {
  d->fields_.emplace(std::move(name), std::move(value));
}

void Header::emplace(const std::string &name,
                     const HeaderValue &value) noexcept {
  d->fields_.emplace(name, value);
}

size_t Header::size() const noexcept { return d->fields_.size(); }

Header::Iterator Header::begin() { return d->fields_.begin(); }
Header::Iterator Header::end() { return d->fields_.end(); }

Header::Iterator Header::begin() const { return d->fields_.begin(); }
Header::Iterator Header::end() const { return d->fields_.end(); }

} // namespace network
