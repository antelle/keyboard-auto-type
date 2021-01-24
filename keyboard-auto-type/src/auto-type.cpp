#include <array>
#include <chrono>
#include <thread>

#include "keyboard-auto-type.h"
#include "utils.h"

namespace keyboard_auto_type {

struct ModifierKeyCode {
    Modifier neutral_mod;
    Modifier right_mod;
    KeyCode left_key;
    KeyCode right_key;
};

constexpr std::array MODIFIERS_KEY_CODES{
    ModifierKeyCode{Modifier::Meta, Modifier::RightMeta, KeyCode::Meta, KeyCode::RightMeta},
    ModifierKeyCode{Modifier::Shift, Modifier::RightShift, KeyCode::Shift, KeyCode::RightShift},
    ModifierKeyCode{Modifier::Alt, Modifier::RightAlt, KeyCode::Alt, KeyCode::RightAlt},
    ModifierKeyCode{Modifier::Ctrl, Modifier::RightCtrl, KeyCode::Ctrl, KeyCode::RightCtrl},
};

AutoTypeResult AutoType::text(std::u32string_view str) {
    if (str.length() == 0) {
        return AutoTypeResult::Ok;
    }

    auto result = ensure_modifier_not_pressed();
    if (result != AutoTypeResult::Ok) {
        return result;
    }

    auto native_keys = os_key_codes_for_chars(str);
    auto length = str.length();

    auto pressed_modifiers = Modifier::None;

    auto tx = begin_batch_text_entry();

    for (size_t i = 0; i < length; i++) {
        auto character = str[i];
        if (!character) {
            return throw_or_return(AutoTypeResult::BadArg,
                                   "Typing a null character is not possible");
        }

        auto native_key_with_modifiers = native_keys[i];

        std::optional<os_key_code_t> code;
        auto modifier = Modifier::None;

        if (native_key_with_modifiers.has_value()) {
            code = native_key_with_modifiers->code;
            modifier = native_key_with_modifiers->modifier;

            for (auto mod_key : MODIFIERS_KEY_CODES) {
                auto mod_check = mod_key.neutral_mod;
                auto is_pressed = (modifier & mod_check) == mod_check;
                auto was_pressed = (pressed_modifiers & mod_check) == mod_check;
                if (is_pressed && !was_pressed) {
                    result = key_move(Direction::Down, mod_check);
                } else if (!is_pressed && was_pressed) {
                    result = key_move(Direction::Up, mod_check);
                }
                if (result != AutoTypeResult::Ok) {
                    return result;
                }
            }

            pressed_modifiers = modifier;
        } else if (pressed_modifiers != Modifier::None) {
            result = key_move(Direction::Up, pressed_modifiers);
            if (result != AutoTypeResult::Ok) {
                return result;
            }
            pressed_modifiers = Modifier::None;
        }

        result = key_move(Direction::Down, character, code, modifier);
        if (result != AutoTypeResult::Ok) {
            return result;
        }

        result = key_move(Direction::Up, character, code, modifier);
        if (result != AutoTypeResult::Ok) {
            return result;
        }
    }

    if (pressed_modifiers != Modifier::None) {
        result = key_move(Direction::Up, pressed_modifiers);
        if (result != AutoTypeResult::Ok) {
            return result;
        }
    }

    tx.done();

    return AutoTypeResult::Ok;
}

AutoTypeResult AutoType::text(std::wstring_view str) {
    std::u32string ustr(str.begin(), str.end());
    return text(ustr);
}

AutoTypeResult AutoType::key_press(KeyCode code, Modifier modifier) {
    auto key_code = os_key_code(code);
    if (!key_code.has_value()) {
        return throw_or_return(AutoTypeResult::BadArg, std::string("Key code ") +
                                                           std::to_string(static_cast<int>(code)) +
                                                           " not supported");
    }

    auto result = key_move(Direction::Down, modifier);
    if (result != AutoTypeResult::Ok) {
        return result;
    }

    result = key_move(Direction::Down, 0, key_code, modifier);
    if (result != AutoTypeResult::Ok) {
        return result;
    }
    result = key_move(Direction::Up, 0, key_code, modifier);
    if (result != AutoTypeResult::Ok) {
        return result;
    }

    result = key_move(Direction::Up, modifier);
    if (result != AutoTypeResult::Ok) {
        return result;
    }

    return AutoTypeResult::Ok;
}

AutoTypeResult AutoType::shortcut(KeyCode code) { return key_press(code, shortcut_modifier()); }

AutoTypeResult AutoType::ensure_modifier_not_pressed() {
    auto start_time = std::chrono::system_clock::now();

    while (true) {
        auto pressed_modifiers = get_pressed_modifiers();
        if (pressed_modifiers == Modifier::None) {
            return AutoTypeResult::Ok;
        }
        if (auto_unpress_modifiers_) {
            for (auto mod_key : MODIFIERS_KEY_CODES) {
                if ((pressed_modifiers & mod_key.neutral_mod) == mod_key.neutral_mod) {
                    if ((pressed_modifiers & mod_key.right_mod) == mod_key.right_mod) {
                        key_move(Direction::Up, mod_key.right_key);
                    } else {
                        key_move(Direction::Up, mod_key.left_key);
                    }
                }
            }
        }
        std::this_thread::sleep_for(KEY_HOLD_LOOP_WAIT_TIME);
        auto elapsed = std::chrono::system_clock::now() - start_time;
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
        if (elapsed_ms > unpress_modifiers_total_wait_time_) {
            break;
        }
    }
    return throw_or_return(AutoTypeResult::ModifierNotReleased, "Modifier key not released");
}

void AutoType::set_auto_unpress_modifiers(bool auto_unpress_modifiers) {
    auto_unpress_modifiers_ = auto_unpress_modifiers;
}

void AutoType::set_unpress_modifiers_total_wait_time(std::chrono::milliseconds time) {
    unpress_modifiers_total_wait_time_ = time;
}

AutoTypeResult AutoType::key_move(Direction direction, KeyCode code, Modifier modifier) {
    auto key_code_opt = os_key_code(code);
    if (!key_code_opt.has_value()) {
        return throw_or_return(AutoTypeResult::BadArg, std::string("Key code ") +
                                                           std::to_string(static_cast<int>(code)) +
                                                           " not supported");
    }
    return key_move(direction, 0, key_code_opt, modifier);
}

AutoTypeResult AutoType::key_move(Direction direction, Modifier modifier) {
    if (modifier == Modifier::None) {
        return AutoTypeResult::Ok;
    }

    for (auto mod_key : MODIFIERS_KEY_CODES) {
        AutoTypeResult result;
        if ((modifier & mod_key.neutral_mod) == mod_key.neutral_mod) {
            if ((modifier & mod_key.right_mod) == mod_key.right_mod) {
                result = key_move(direction, mod_key.right_key);
            } else {
                result = key_move(direction, mod_key.left_key);
            }
            if (result != AutoTypeResult::Ok) {
                return result;
            }
        }
    }

    return AutoTypeResult::Ok;
}

AutoTypeTextTransaction::AutoTypeTextTransaction(std::function<void()> end_callback)
    : end_callback_(std::move(end_callback)) {}

AutoTypeTextTransaction::~AutoTypeTextTransaction() { done(); }

void AutoTypeTextTransaction::done() noexcept {
    if (end_callback_) {
#if __cpp_exceptions
        try {
#endif
            end_callback_();
#if __cpp_exceptions
        } catch (...) {
        }
#endif
        end_callback_ = nullptr;
    }
}

} // namespace keyboard_auto_type
