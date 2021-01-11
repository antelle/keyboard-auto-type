#ifndef __KEYBOARD_AUTO_TYPE_H__
#define __KEYBOARD_AUTO_TYPE_H__

#include <cstdint>
#include <memory>
#include <string>

#include "key-code.h"

namespace keyboard_auto_type {

enum class Modifier : uint8_t {
    None = 0b0000,

    Ctrl = 0b0001,
    Control = 0b0001,

    Alt = 0b0010,
    Option = 0b0010,

    Shift = 0b0100,

    Meta = 0b1000,
    Command = 0b1000,
    Win = 0b1000,
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

class AutoType {
  private:
    AutoType(const AutoType &) = delete;
    AutoType &operator=(const AutoType &) = delete;
    AutoType(AutoType &&) = delete;
    AutoType &operator=(AutoType &&) = delete;

    class AutoTypeImpl;
    std::unique_ptr<AutoTypeImpl> impl_;

  public:
    AutoType();
    ~AutoType();

    AutoTypeResult key_move(Direction direction, wchar_t character, Modifier modifier = Modifier::None);
    AutoTypeResult key_move(Direction direction, wchar_t character, KeyCode code, Modifier modifier = Modifier::None);
    AutoTypeResult key_move(Direction direction, Modifier modifier);

    AutoTypeResult key_press(wchar_t character, Modifier modifier = Modifier::None);
    AutoTypeResult key_press(wchar_t character, KeyCode code, Modifier modifier = Modifier::None);

    AutoTypeResult text(std::wstring text, Modifier modifier = Modifier::None);

    AutoTypeResult press_copy();
    AutoTypeResult press_paste();
    AutoTypeResult press_cut();

    static Modifier shortcut_modifier();
};

} // namespace keyboard_auto_type

#endif
