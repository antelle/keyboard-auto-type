#include <algorithm>
#include <array>
#include <iostream>
#include <stdexcept>
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

AutoTypeResult AutoType::key_move(Direction direction, char32_t character, os_key_code_t code,
                                  Modifier modifier) {
    if (character > MAXWORD) {
        return AutoTypeResult::BadArg;
    }
    // auto flags = direction == Direction::Up ? KEYEVENTF_KEYUP : 0;
    // keybd_event(code, 0, flags, NULL);

    KEYBDINPUT keyboard_input{};
    keyboard_input.wVk = code;
    keyboard_input.wScan = static_cast<WORD>(character);
    keyboard_input.dwFlags = direction == Direction::Up ? KEYEVENTF_KEYUP : 0;

    if (character) {
        keyboard_input.wVk = 0;
        keyboard_input.dwFlags |= KEYEVENTF_UNICODE;
    }

    INPUT input = {INPUT_KEYBOARD};
    input.ki = keyboard_input;

    auto events_sent = SendInput(1, &input, sizeof(input));

    if (!events_sent) {
#if __cpp_exceptions && !defined(KEYBOARD_AUTO_TYPE_NO_EXCEPTIONS)
        throw std::runtime_error("SendInput error");
#endif
        return AutoTypeResult::OsError;
    }

    return AutoTypeResult::Ok;
}

Modifier AutoType::get_pressed_modifiers() {
    static constexpr std::array FLAGS_MODIFIERS{
        std::make_pair(VK_LWIN, Modifier::Command),    std::make_pair(VK_RWIN, Modifier::Command),
        std::make_pair(VK_SHIFT, Modifier::Shift),     std::make_pair(VK_MENU, Modifier::Option),
        std::make_pair(VK_CONTROL, Modifier::Control),
    };
    static constexpr SHORT SHORT_MSB = 0b10000000'00000000;
    auto pressed_modifiers = Modifier::None;
    for (auto [key_code, modifier] : FLAGS_MODIFIERS) {
        auto key_state = GetAsyncKeyState(key_code);
        if (key_state & SHORT_MSB) {
            pressed_modifiers = pressed_modifiers | modifier;
        }
    }
    return pressed_modifiers;
}

bool AutoType::can_unpress_modifier() { return false; }

Modifier AutoType::shortcut_modifier() { return Modifier::Control; }

std::optional<os_key_code_t> AutoType::os_key_code(KeyCode code) {
    if (code == KeyCode::Undefined) {
        return std::nullopt;
    }
    auto native_key_code = map_key_code(code);
    if (!native_key_code) {
        return std::nullopt;
    }
    return native_key_code;
}

os_key_code_t AutoType::os_key_code_for_char(char32_t character) { return 0; }

std::vector<os_key_code_t> AutoType::os_key_codes_for_chars(std::u32string_view text) {
    return std::vector<os_key_code_t>(text.length());
}

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
    result.window_id = hwnd;
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
            if (GetClassNameA(hwnd, window_class_name.data(),
                              static_cast<int>(window_class_name.size()))) {
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

bool AutoType::show_window(const AppWindowInfo &window) {
    if (!window.window_id) {
        return false;
    }

    auto current_window = GetForegroundWindow();
    auto current_thread_id = GetCurrentThreadId();
    auto win_thread_id = GetWindowThreadProcessId(current_window, 0);

    if (current_thread_id != win_thread_id) {
        AttachThreadInput(current_thread_id, win_thread_id, TRUE);

        DWORD lock_timeout = 0;
        SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &lock_timeout, 0);
        SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, 0,
                             SPIF_SENDWININICHANGE | SPIF_UPDATEINIFILE);

        AllowSetForegroundWindow(ASFW_ANY);
    }

    auto result = SetForegroundWindow(window.window_id);

    if (current_thread_id != win_thread_id) {
        DWORD lock_timeout = 0;
        SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, &lock_timeout,
                             SPIF_SENDWININICHANGE | SPIF_UPDATEINIFILE);
        AttachThreadInput(current_thread_id, win_thread_id, FALSE);
    }
    return result;
}

} // namespace keyboard_auto_type
