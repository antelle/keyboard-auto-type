#pragma once

#include <Carbon/Carbon.h>

#include <string>

namespace keyboard_auto_type {

std::string cfstring_to_string(CFStringRef str);
int get_number_from_dictionary(CFDictionaryRef dictionary, CFStringRef key);
std::string get_string_from_dictionary(CFDictionaryRef dictionary, CFStringRef key);
std::string ax_get_focused_window_title(pid_t pid);

} // namespace keyboard_auto_type
