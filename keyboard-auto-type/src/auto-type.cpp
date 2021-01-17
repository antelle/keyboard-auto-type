#include <array>
#include <chrono>
#include <stdexcept>
#include <thread>

#include "keyboard-auto-type.h"

namespace keyboard_auto_type {

constexpr int KEY_HOLD_TOTAL_WAIT_TIME_MS = 10'000;
constexpr int KEY_HOLD_LOOP_WAIT_TIME_MS = 100;

constexpr std::array MODIFIERS{Modifier::Meta, Modifier::Shift, Modifier::Alt, Modifier::Ctrl};
constexpr std::array MODIFIERS_KEY_CODES{
    std::make_pair(Modifier::Meta, KeyCode::Meta),
    std::make_pair(Modifier::Shift, KeyCode::Shift),
    std::make_pair(Modifier::Alt, KeyCode::Alt),
    std::make_pair(Modifier::Ctrl, KeyCode::Ctrl),
};

AutoTypeResult AutoType::text(std::u32string_view text) {
    if (text.length() == 0) {
        return AutoTypeResult::Ok;
    }

    auto result = ensure_modifier_not_pressed();
    if (result != AutoTypeResult::Ok) {
        return result;
    }

    auto native_key_codes = os_key_codes_for_chars(text);
    auto length = text.length();

    for (auto i = 0; i < length; i++) {
        auto character = text[i];
        auto native_key_code = native_key_codes[i];
        result = key_move(Direction::Down, character, native_key_code);
        if (result != AutoTypeResult::Ok) {
            return result;
        }
        result = key_move(Direction::Up, character, native_key_code);
    }

    return result;
}

AutoTypeResult AutoType::key_press(char32_t character, KeyCode code, Modifier modifier) {
    if (!character && code == KeyCode::Undefined) {
#if __cpp_exceptions && !defined(KEYBOARD_AUTO_TYPE_NO_EXCEPTIONS)
        throw std::invalid_argument("Either character or code must be set");
#endif
        return AutoTypeResult::BadArg;
    }

    auto result = ensure_modifier_not_pressed();
    if (result != AutoTypeResult::Ok) {
        return result;
    }

    os_key_code_t native_key_code = 0;
    if (code == KeyCode::Undefined) {
        native_key_code = os_key_code_for_char(character);
    } else {
        auto os_key_code_opt = os_key_code(code);
        if (!os_key_code_opt.has_value()) {
#if __cpp_exceptions && !defined(KEYBOARD_AUTO_TYPE_NO_EXCEPTIONS)
            throw std::invalid_argument(std::string("Key code ") +
                                        std::to_string(static_cast<int>(code)) + " not supported");
#endif
        }
        native_key_code = os_key_code_opt.value();
    }

    if (modifier != Modifier::None) {
        result = key_move(Direction::Down, modifier);
        if (result != AutoTypeResult::Ok) {
            return result;
        }
    }

    result = key_move(Direction::Down, character, native_key_code, modifier);
    if (result != AutoTypeResult::Ok) {
        return result;
    }
    result = key_move(Direction::Up, character, native_key_code, modifier);
    if (result != AutoTypeResult::Ok) {
        return result;
    }

    if (modifier != Modifier::None) {
        result = key_move(Direction::Up, modifier);
    }

    return result;
}

AutoTypeResult AutoType::key_press(char32_t character, Modifier modifier) {
    return key_press(character, KeyCode::Undefined, modifier);
}

AutoTypeResult AutoType::shortcut(KeyCode code) { return key_press(0, code, shortcut_modifier()); }

AutoTypeResult AutoType::ensure_modifier_not_pressed() {
    auto total_wait_time = KEY_HOLD_TOTAL_WAIT_TIME_MS;
    auto loop_wait_time = KEY_HOLD_LOOP_WAIT_TIME_MS;

    while (total_wait_time > 0) {
        auto pressed_modifiers = get_pressed_modifiers();
        if (pressed_modifiers == Modifier::None) {
            return AutoTypeResult::Ok;
        }
        if (can_unpress_modifier()) {
            for (auto modifier : MODIFIERS) {
                if ((pressed_modifiers & modifier) == modifier) {
                    key_move(Direction::Up, modifier);
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(loop_wait_time));
        total_wait_time -= loop_wait_time;
    }
#if __cpp_exceptions && !defined(KEYBOARD_AUTO_TYPE_NO_EXCEPTIONS)
    throw std::runtime_error("Modifier key not released");
#endif
    return AutoTypeResult::ModifierNotReleased;
}

AutoTypeResult AutoType::key_move(Direction direction, char32_t character, Modifier modifier) {
    return key_move(direction, character, KeyCode::Undefined, modifier);
}

AutoTypeResult AutoType::key_move(Direction direction, KeyCode code, Modifier modifier) {
    return key_move(direction, 0, code, modifier);
}

AutoTypeResult AutoType::key_move(Direction direction, Modifier modifier) {
    if (modifier == Modifier::None) {
        return AutoTypeResult::Ok;
    }

    for (auto [mod_check, key_code] : MODIFIERS_KEY_CODES) {
        AutoTypeResult result;
        if ((modifier & mod_check) == mod_check) {
            result = key_move(direction, 0, key_code);
            if (result != AutoTypeResult::Ok) {
                return result;
            }
        }
    }

    return AutoTypeResult::Ok;
}

AutoTypeResult AutoType::key_move(Direction direction, char32_t character, KeyCode code,
                                  Modifier modifier) {
    auto native_key_code = os_key_code(code);
    if (code != KeyCode::Undefined && !native_key_code.has_value()) {
#if __cpp_exceptions && !defined(KEYBOARD_AUTO_TYPE_NO_EXCEPTIONS)
        throw std::invalid_argument(std::string("Key code ") +
                                    std::to_string(static_cast<int>(code)) + " not supported");
#endif
        return AutoTypeResult::NotSupported;
    }
    return key_move(direction, character, native_key_code.value(), modifier);
}

} // namespace keyboard_auto_type
