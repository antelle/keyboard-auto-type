#pragma once

#include <unistd.h>

#include <string>

namespace keyboard_auto_type {

std::string get_browser_url_using_atspi(pid_t pid);

} // namespace keyboard_auto_type
