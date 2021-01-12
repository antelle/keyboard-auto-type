#include "keyboard-auto-type.h"

namespace keyboard_auto_type {

AutoTypeResult AutoType::key_move(Direction direction, wchar_t character, Modifier modifier) {
    return key_move(direction, character, KeyCode::Undefined, modifier);
}

AutoTypeResult AutoType::key_press(wchar_t character, Modifier modifier) {
    return key_press(character, KeyCode::Undefined, modifier);
}

AutoTypeResult AutoType::press_copy() { return key_press(0, KeyCode::ANSI_C, shortcut_modifier()); }

AutoTypeResult AutoType::press_paste() {
    return key_press(0, KeyCode::ANSI_V, shortcut_modifier());
}

AutoTypeResult AutoType::press_cut() { return key_press(0, KeyCode::ANSI_X, shortcut_modifier()); }

} // namespace keyboard_auto_type
