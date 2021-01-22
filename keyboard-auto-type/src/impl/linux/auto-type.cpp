#include <iostream>

#include "key-map.h"
#include "keyboard-auto-type.h"
#include "utils.h"

namespace keyboard_auto_type {

class AutoType::AutoTypeImpl {
  private:
  public:
    AutoTypeResult key_move(Direction direction, char32_t character, os_key_code_t code) {
        auto down = direction == Direction::Down;
        std::cout << "move " << down << " " << character << " " << code << std::endl;
        return AutoTypeResult::Ok;
    }
};

AutoType::AutoType() : impl_(std::make_unique<AutoType::AutoTypeImpl>()) {}

AutoType::~AutoType() = default;

AutoTypeResult AutoType::key_move(Direction direction, char32_t character,
                                  std::optional<os_key_code_t> code, Modifier modifier) {
    // return impl_->key_move(direction, character, code.value_or(0));
    return AutoTypeResult::Ok;
}

Modifier AutoType::get_pressed_modifiers() {
    auto pressed_modifiers = Modifier::None;
    return pressed_modifiers;
}

Modifier AutoType::shortcut_modifier() { return Modifier::Ctrl; }

std::optional<os_key_code_t> AutoType::os_key_code(KeyCode code) {
    if (code == KeyCode::Undefined) {
        return std::nullopt;
    }
    auto native_key_code = map_key_code(code);
    return native_key_code;
}

std::optional<KeyCodeWithModifiers> AutoType::os_key_code_for_char(char32_t character) {
    return std::nullopt;
}

std::vector<std::optional<KeyCodeWithModifiers>>
AutoType::os_key_codes_for_chars(std::u32string_view text) {
    std::vector<std::optional<KeyCodeWithModifiers>> result(text.length());
    auto length = text.length();
    for (size_t i = 0; i < length; i++) {
        result[i] = std::nullopt;
    }
    return result;
}

pid_t AutoType::active_pid() { return 0; }

AppWindow AutoType::active_window(const ActiveWindowArgs &args) { return {}; }

bool AutoType::show_window(const AppWindow &window) {
    if (!window.pid) {
        return false;
    }
    return false;
}

} // namespace keyboard_auto_type
