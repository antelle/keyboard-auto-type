#ifndef KEYBOARD_AUTO_TYPE_H
#define KEYBOARD_AUTO_TYPE_H

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "key-code.h"

namespace keyboard_auto_type {

#if __APPLE__
using os_key_code_t = uint16_t;
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
using pid_t = unsigned long;
using os_key_code_t = uint16_t;
#else
using os_key_code_t = uint32_t;
#endif

enum class Modifier : uint8_t {
    None = 0b0000,

    Ctrl = 0b0001,
    Control = Ctrl,
    RightCtrl = 0b0001'0001,
    RightControl = RightCtrl,
    LeftCtrl = 0b0010'0001,
    LeftControl = LeftCtrl,

    Alt = 0b0010,
    Option = Alt,
    RightAlt = 0b0001'0010,
    RightOption = RightAlt,
    LeftAlt = 0b0010'0010,
    LeftOption = LeftAlt,

    Shift = 0b0100,
    RightShift = 0b0001'0100,
    LeftShift = 0b0010'0100,

    Meta = 0b1000,
    Command = Meta,
    Win = Meta,
    RightMeta = 0b0001'1000,
    RightCommand = RightMeta,
    RightWin = RightMeta,
    LeftMeta = 0b0010'1000,
    LeftCommand = LeftMeta,
    LeftWin = LeftMeta,
};

constexpr Modifier operator|(Modifier lhs, Modifier rhs) {
    return static_cast<Modifier>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}

constexpr Modifier operator&(Modifier lhs, Modifier rhs) {
    return static_cast<Modifier>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
}

enum class Direction { Up, Down };

enum class AutoTypeResult {
    Ok,
    BadArg,
    ModifierNotReleased,
    KeyPressFailed,
    NotSupported,
    OsError,
};

struct AppWindow {
    pid_t pid = 0;
    void *window_id = nullptr;
    std::string app_name;
    std::string title;
    std::string url;
};

struct ActiveWindowArgs {
    bool get_window_title = false;
    bool get_browser_url = false;
};

struct KeyCodeWithModifiers {
    os_key_code_t code;
    Modifier modifier;
};

class AutoType {
  private:
    static constexpr auto DEFAULT_UNPRESS_MODIFIERS_TOTAL_WAIT_TIME =
        std::chrono::milliseconds(10'000);
    static constexpr auto KEY_HOLD_LOOP_WAIT_TIME = std::chrono::milliseconds(100);

    class AutoTypeImpl;
    std::unique_ptr<AutoTypeImpl> impl_;

    bool auto_unpress_modifiers_ = true;
    std::chrono::milliseconds unpress_modifiers_total_wait_time_ =
        DEFAULT_UNPRESS_MODIFIERS_TOTAL_WAIT_TIME;

  public:
    AutoType();
    ~AutoType();

    AutoType(const AutoType &) = delete;
    AutoType &operator=(const AutoType &) = delete;
    AutoType(AutoType &&) = delete;
    AutoType &operator=(AutoType &&) = delete;

    AutoTypeResult text(std::u32string_view str);
    AutoTypeResult text(std::wstring_view str);

    AutoTypeResult key_press(KeyCode code, Modifier modifier = Modifier::None);

    AutoTypeResult shortcut(KeyCode code);
    static Modifier shortcut_modifier();

    AutoTypeResult ensure_modifier_not_pressed();
    Modifier get_pressed_modifiers();
    void set_auto_unpress_modifiers(bool auto_unpress_modifiers);
    void set_unpress_modifiers_total_wait_time(std::chrono::milliseconds time);

    AutoTypeResult key_move(Direction direction, KeyCode code, Modifier modifier = Modifier::None);
    AutoTypeResult key_move(Direction direction, Modifier modifier);
    AutoTypeResult key_move(Direction direction, char32_t character,
                            std::optional<os_key_code_t> code, Modifier modifier = Modifier::None);

    std::optional<os_key_code_t> os_key_code(KeyCode code);
    std::optional<KeyCodeWithModifiers> os_key_code_for_char(char32_t character);
    std::vector<std::optional<KeyCodeWithModifiers>>
    os_key_codes_for_chars(std::u32string_view text);

    static pid_t active_pid();
    static AppWindow active_window(const ActiveWindowArgs &args = {});
    static bool show_window(const AppWindow &window);
};

} // namespace keyboard_auto_type

#endif
