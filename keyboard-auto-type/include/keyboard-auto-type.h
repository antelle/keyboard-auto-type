#ifndef __KEYBOARD_AUTO_TYPE_H__
#define __KEYBOARD_AUTO_TYPE_H__

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#include "key-code.h"

namespace keyboard_auto_type {

enum class Modifier : uint8_t {
    None = 0b0000,

    Ctrl = 0b0001,
    Control = Ctrl,

    Alt = 0b0010,
    Option = Alt,

    Shift = 0b0100,

    Meta = 0b1000,
    Command = Meta,
    Win = Meta,
};

Modifier operator+(Modifier lhs, Modifier rhs);
Modifier operator|(Modifier lhs, Modifier rhs);
Modifier operator&(Modifier lhs, Modifier rhs);

enum class Direction : uint8_t { Up, Down };

enum class AutoTypeResult : uint8_t {
    Ok,
    BadArg,
    ModifierNotReleased,
};

struct AppWindowInfo {
    uint64_t pid = 0;
    intptr_t window_id = 0;
    std::string process_name;
    std::string title;
    std::string url;
};

enum class ErrorMode : uint8_t { Throw, Return };

class AutoType {
  private:
    static ErrorMode error_mode_;
    class AutoTypeImpl;
    std::unique_ptr<AutoTypeImpl> impl_;

  public:
    AutoType();
    ~AutoType();

    AutoType(const AutoType &) = delete;
    AutoType &operator=(const AutoType &) = delete;
    AutoType(AutoType &&) = delete;
    AutoType &operator=(AutoType &&) = delete;

    static void set_error_mode(ErrorMode error_mode);

    AutoTypeResult key_move(Direction direction, char32_t character,
                            Modifier modifier = Modifier::None);
    AutoTypeResult key_move(Direction direction, char32_t character, KeyCode code,
                            Modifier modifier = Modifier::None);
    AutoTypeResult key_move(Direction direction, Modifier modifier);

    AutoTypeResult key_press(char32_t character, Modifier modifier = Modifier::None);
    AutoTypeResult key_press(char32_t character, KeyCode code, Modifier modifier = Modifier::None);

    AutoTypeResult text(std::u32string_view text, Modifier modifier = Modifier::None);

    AutoTypeResult shortcut(KeyCode code);

    static Modifier shortcut_modifier();

    static AppWindowInfo active_window();
    static void show_window(const AppWindowInfo &window);
    static void show_self();
    static void hide_self();
};

} // namespace keyboard_auto_type

#endif
