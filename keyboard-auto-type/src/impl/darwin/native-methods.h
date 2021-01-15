#pragma once

#include <CoreFoundation/CoreFoundation.h>

#include <string>

namespace keyboard_auto_type {

struct NativeAppInfo {
    pid_t pid;
    std::string name;
    std::string bundle_id;
};

struct NativeWindowInfo {
    std::string title;
    std::string url;
};

pid_t native_frontmost_app_pid();
NativeAppInfo native_frontmost_app();
bool native_show_app(pid_t pid);
NativeWindowInfo native_window_info(pid_t pid);

} // namespace keyboard_auto_type
