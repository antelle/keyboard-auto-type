#include "keyboard-auto-type.h"

namespace keyboard_auto_type {

AutoTypeResult AutoType::key_press(char32_t character, Modifier modifier) {
    return key_press(character, KeyCode::Undefined, modifier);
}

AutoTypeResult AutoType::shortcut(KeyCode code) { return key_press(0, code, shortcut_modifier()); }

AutoTypeResult AutoType::key_move(Direction direction, char32_t character, Modifier modifier) {
    return key_move(direction, character, KeyCode::Undefined, modifier);
}

} // namespace keyboard_auto_type
