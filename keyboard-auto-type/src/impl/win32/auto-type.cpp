#include <algorithm>
#include <array>
#include <functional>
#include <vector>

#include "key-map.h"
#include "keyboard-auto-type.h"
#include "utils.h"
#include "winapi-tools.h"

namespace keyboard_auto_type {

constexpr std::array BROWSER_PROCESS_NAMES{
    "chrome", "firefox", "opera", "browser", "applicationframehost", "iexplore", "edge"};
constexpr std::string_view BROWSER_WINDOW_CLASS = "Chrome_WidgetWin_1";
static constexpr SHORT SHORT_MSB = static_cast<SHORT>(0b10000000'00000000);
static const auto EXTENDED_KEYS = std::invoke([] {
    std::array<bool, 0xFF> extended_keys;
    extended_keys.at(VK_MENU) = true;
    extended_keys.at(VK_RMENU) = true;
    extended_keys.at(VK_CONTROL) = true;
    extended_keys.at(VK_RCONTROL) = true;
    extended_keys.at(VK_INSERT) = true;
    extended_keys.at(VK_DELETE) = true;
    extended_keys.at(VK_HOME) = true;
    extended_keys.at(VK_END) = true;
    extended_keys.at(VK_PRIOR) = true;
    extended_keys.at(VK_NEXT) = true;
    extended_keys.at(VK_DOWN) = true;
    extended_keys.at(VK_LEFT) = true;
    extended_keys.at(VK_RIGHT) = true;
    extended_keys.at(VK_UP) = true;
    extended_keys.at(VK_NUMLOCK) = true;
    extended_keys.at(VK_CANCEL) = true;
    extended_keys.at(VK_SNAPSHOT) = true;
    extended_keys.at(VK_DIVIDE) = true;
    return extended_keys;
});

class AutoType::AutoTypeImpl {
  public:
    static HKL active_layout() {
        return GetKeyboardLayout(GetWindowThreadProcessId(GetForegroundWindow(), nullptr));
    }

    static std::optional<KeyCodeWithModifiers> char_to_key_code(HKL layout, char32_t character) {
        if (character > WCHAR_MAX) {
            return std::nullopt;
        }
        auto scan_code_ex = VkKeyScanEx(static_cast<WCHAR>(character), layout);

        auto vk = LOBYTE(scan_code_ex);
        auto shift_state = HIBYTE(scan_code_ex);

        if (!vk || vk == static_cast<BYTE>(-1) || shift_state == static_cast<BYTE>(-1)) {
            return std::nullopt;
        }

        KeyCodeWithModifiers res{};
        res.code = vk;

        static constexpr std::array SHIFT_STATES_MODIFIERS{
            std::make_pair(1, Modifier::Shift),
            std::make_pair(2, Modifier::Ctrl),
            std::make_pair(4, Modifier::Alt),
        };

        for (auto [shift_state_mask, modifier] : SHIFT_STATES_MODIFIERS) {
            if (shift_state & shift_state_mask) {
                res.modifier = res.modifier | modifier;
            }
        }

        return res;
    };
};

AutoType::AutoType() : impl_(std::make_unique<AutoType::AutoTypeImpl>()) {}

AutoType::~AutoType() = default;

AutoTypeResult AutoType::key_move(Direction direction, char32_t character,
                                  std::optional<os_key_code_t> code, Modifier) {
    auto down = direction == Direction::Down;

    KEYBDINPUT keyboard_input{};
    keyboard_input.dwFlags = down ? 0 : KEYEVENTF_KEYUP;
    keyboard_input.dwExtraInfo = GetMessageExtraInfo();

    if (code.has_value()) {
        keyboard_input.wVk = code.value();
        if (code.value() < EXTENDED_KEYS.size() && EXTENDED_KEYS.at(code.value())) {
            keyboard_input.dwFlags |= KEYEVENTF_EXTENDEDKEY;
        }
        auto scan_code = MapVirtualKey(code.value(), MAPVK_VK_TO_VSC);
        keyboard_input.wScan = LOWORD(scan_code);
    } else {
        keyboard_input.wVk = 0;
        keyboard_input.wScan = static_cast<WORD>(character);
        keyboard_input.dwFlags |= KEYEVENTF_UNICODE;
    }

    INPUT input = {INPUT_KEYBOARD};
    input.ki = keyboard_input;

    auto events_sent = SendInput(1, &input, sizeof(input));

    if (!events_sent) {
        return throw_or_return(AutoTypeResult::OsError, "SendInput error");
    }

    return AutoTypeResult::Ok;
}

Modifier AutoType::get_pressed_modifiers() {
    static constexpr std::array FLAGS_MODIFIERS{
        std::make_pair(VK_LWIN, Modifier::Command),    std::make_pair(VK_RWIN, Modifier::Command),
        std::make_pair(VK_SHIFT, Modifier::Shift),     std::make_pair(VK_MENU, Modifier::Option),
        std::make_pair(VK_CONTROL, Modifier::Control),
    };
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

std::optional<KeyCodeWithModifiers> AutoType::os_key_code_for_char(char32_t character) {
    return impl_->char_to_key_code(impl_->active_layout(), character);
}

std::vector<std::optional<KeyCodeWithModifiers>>
AutoType::os_key_codes_for_chars(std::u32string_view text) {
    auto layout = impl_->active_layout();
    std::vector<std::optional<KeyCodeWithModifiers>> result(text.length());
    auto length = text.length();
    for (auto i = 0; i < length; i++) {
        result[i] = impl_->char_to_key_code(layout, text[i]);
    }
    return result;
}

pid_t AutoType::active_pid() {
    DWORD pid = 0;
    auto hwnd = GetForegroundWindow();
    if (hwnd) {
        GetWindowThreadProcessId(hwnd, &pid);
    }
    return pid;
}

AppWindow AutoType::active_window(const ActiveWindowArgs &args) {
    auto hwnd = GetForegroundWindow();
    if (!hwnd) {
        return {};
    }

    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);

    AppWindow result{};
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
            result.url = native_browser_url(hwnd);
        }
    }

    return result;
}

bool AutoType::show_window(const AppWindow &window) {
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
