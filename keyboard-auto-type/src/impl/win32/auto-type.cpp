#include <array>
#include <codecvt>
#include <exception>
#include <iostream>

#include "key-map.h"
#include "keyboard-auto-type.h"

namespace keyboard_auto_type {

static constexpr int KEY_HOLD_TOTAL_WAIT_TIME = 10 * 1000 * 1000;
static constexpr int KEY_HOLD_LOOP_WAIT_TIME = 10000;
static constexpr int MAX_KEYBOARD_LAYOUT_CHAR_CODE = 128;

class AutoType::AutoTypeImpl {
};

AutoType::AutoType() : impl_(std::make_unique<AutoType::AutoTypeImpl>()) {}

AutoType::~AutoType() = default;

AutoTypeResult AutoType::key_move(Direction direction, char32_t character, KeyCode code,
                                  Modifier modifier) {
    return AutoTypeResult::Ok;
}

AutoTypeResult AutoType::ensure_modifier_not_pressed() {
    return AutoTypeResult::Ok;
}

AutoTypeResult AutoType::key_move(Direction direction, Modifier modifier) {
    return AutoTypeResult::Ok;
}

AutoTypeResult AutoType::key_press(char32_t character, KeyCode code, Modifier modifier) {
    return AutoTypeResult::Ok;
}

AutoTypeResult AutoType::text(std::u32string_view text, Modifier modifier) {
    return AutoTypeResult::Ok;
}

Modifier AutoType::shortcut_modifier() { return Modifier::Control; }

pid_t AutoType::active_pid() { return 0; }

AppWindowInfo AutoType::active_window(const ActiveWindowArgs &args) { return {}; }

bool AutoType::show_window(const AppWindowInfo &window) { return false; }

} // namespace keyboard_auto_type
