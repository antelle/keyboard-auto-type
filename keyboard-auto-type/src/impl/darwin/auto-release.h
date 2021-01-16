#pragma once

#include <CoreFoundation/CoreFoundation.h>

namespace keyboard_auto_type {

template <typename T> class auto_release {
  private:
    T resource_ = nullptr;

  public:
    auto_release() { resource_ = nullptr; }

    // cppcheck-suppress noExplicitConstructor
    auto_release(T resource) // NOLINT(google-explicit-constructor, hicpp-explicit-conversions)
        : resource_(resource) {}

    auto_release(const auto_release &) = delete;
    auto_release &operator=(const auto_release &other) = delete;

    auto_release(auto_release &&other) noexcept : resource_(other.resource_) {
        other.resource_ = nullptr;
    }

    auto_release &operator=(auto_release &&other) noexcept {
        resource_ = other.resource;
        other.resource_ = nullptr;
        return *this;
    }

    ~auto_release() {
        if (resource_) {
            CFRelease(resource_);
            resource_ = nullptr;
        }
    }

    operator T() const { // NOLINT(google-explicit-constructor, hicpp-explicit-conversions)
        return resource_;
    }

    T *operator&() { return &resource_; } // NOLINT(google-runtime-operator)
};

} // namespace keyboard_auto_type
