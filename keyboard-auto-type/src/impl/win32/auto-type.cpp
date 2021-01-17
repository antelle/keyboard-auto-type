#include <algorithm>
#include <array>
#include <exception>
#include <iostream>
#include <vector>

#include "key-map.h"
#include "keyboard-auto-type.h"
#include "winapi-tools.h"

namespace keyboard_auto_type {

constexpr std::array BROWSER_PROCESS_NAMES{
    "chrome", "firefox", "opera", "browser", "applicationframehost", "iexplore", "edge"};
constexpr std::string_view BROWSER_WINDOW_CLASS = "Chrome_WidgetWin_1";

class AutoType::AutoTypeImpl {};

AutoType::AutoType() : impl_(std::make_unique<AutoType::AutoTypeImpl>()) {}

AutoType::~AutoType() = default;

// AutoTypeResult AutoType::ensure_modifier_not_pressed() {
//     auto total_wait_time = KEY_HOLD_TOTAL_WAIT_TIME;
//     auto loop_wait_time = KEY_HOLD_LOOP_WAIT_TIME;
//     static constexpr std::array checked_keys = {
//         VK_SHIFT, VK_RSHIFT, VK_CONTROL, VK_RCONTROL, VK_MENU, VK_RMENU, VK_LWIN, VK_RWIN,
//     };
//     while (total_wait_time > 0) {
//         auto any_pressed = false;
//         for (auto key : checked_keys) {
//             auto key_state = GetKeyState(key);
//             if (key_state) {
//                 // TODO: press it?
//                 any_pressed = true;
//                 break;
//             }
//         }
//         if (!any_pressed) {
//             return AutoTypeResult::Ok;
//         }
//         Sleep(loop_wait_time);
//         total_wait_time -= loop_wait_time;
//     }
// #if __cpp_exceptions && !defined(KEYBOARD_AUTO_TYPE_NO_EXCEPTIONS)
//     throw std::runtime_error("Modifier key not released");
// #endif
//     return AutoTypeResult::ModifierNotReleased;
// }

Modifier AutoType::shortcut_modifier() { return Modifier::Control; }

pid_t AutoType::active_pid() {
    DWORD pid = 0;
    auto hwnd = GetForegroundWindow();
    if (hwnd) {
        GetWindowThreadProcessId(hwnd, &pid);
    }
    return pid;
}

AppWindowInfo AutoType::active_window(const ActiveWindowArgs &args) {
    auto hwnd = GetForegroundWindow();
    if (!hwnd) {
        return {};
    }

    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);

    AppWindowInfo result{};
    result.window_id = reinterpret_cast<intptr_t>(hwnd);
    result.pid = pid;

    result.app_name = native_process_main_module_name(pid);
    if (args.get_window_title) {
        result.title = native_window_text(hwnd);
    }
    if (args.get_browser_url) {
        auto is_browser = std::any_of(BROWSER_PROCESS_NAMES.begin(), BROWSER_PROCESS_NAMES.end(),
                                      [&app_name = result.app_name](auto name) {
                                          return includes_case_insensitive(app_name, name);
                                      });
        if (!is_browser) {
            std::array<char, BROWSER_WINDOW_CLASS.size() + 1> window_class_name;
            if (GetClassNameA(hwnd, window_class_name.data(), window_class_name.size())) {
                if (BROWSER_WINDOW_CLASS == std::string_view(window_class_name.data())) {
                    is_browser = true;
                }
            }
        }
        if (is_browser) {
            result.url = native_browser_url(pid, hwnd);
        }
    }

    return result;
}

bool AutoType::show_window(const AppWindowInfo &window) { return false; }

} // namespace keyboard_auto_type
