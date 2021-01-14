#include "keyboard-auto-type.h"

namespace keyboard_auto_type {

#if __cpp_exceptions
bool AutoType::throw_exceptions_ = true;
#else
bool AutoType::throw_exceptions_ = false;
#endif

void AutoType::set_throw_exceptions(bool throw_exceptions) { throw_exceptions_ = throw_exceptions; }

AutoTypeResult AutoType::key_move(Direction direction, char32_t character, Modifier modifier) {
    return key_move(direction, character, KeyCode::Undefined, modifier);
}

AutoTypeResult AutoType::key_press(char32_t character, Modifier modifier) {
    return key_press(character, KeyCode::Undefined, modifier);
}

AutoTypeResult AutoType::shortcut(KeyCode code) { return key_press(0, code, shortcut_modifier()); }

} // namespace keyboard_auto_type
