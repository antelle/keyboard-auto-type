#pragma once

#include <Windows.h>
#include <string>

namespace keyboard_auto_type {

std::string winapi_to_string(LPCWSTR api_str);
bool includes_case_insensitive(std::string_view str, std::string_view search);
std::string native_window_text(HWND hwnd);
std::string native_process_main_module_name(DWORD pid);
std::string native_browser_url(DWORD pid, HWND hwnd);

} // namespace keyboard_auto_type
