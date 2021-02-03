#include <Carbon/Carbon.h>

#include <array>
#include <chrono>
#include <thread>
#include <unordered_map>

#include "auto-release.h"
#include "carbon-helpers.h"
#include "key-map.h"
#include "keyboard-auto-type.h"
#include "native-methods.h"
#include "utils.h"

namespace keyboard_auto_type {

constexpr auto KEY_PRESS_TOTAL_WAIT_TIME_MS = 10'000;
constexpr auto KEY_PRESS_LOOP_WAIT_TIME_MS = 1;

constexpr std::array BROWSER_APP_BUNDLE_IDS{
    "com.google.Chrome",
    "org.chromium.Chromium",
    "com.google.Chrome.canary",
    "com.apple.Safari",
    "com.apple.SafariTechnologyPreview",
};

constexpr std::array KEY_CHECK_CODES{
    std::make_pair(kVK_RightCommand, kVK_Command),
    std::make_pair(kVK_RightShift, kVK_Shift),
    std::make_pair(kVK_RightOption, kVK_Option),
    std::make_pair(kVK_RightControl, kVK_Control),
};

constexpr std::array EVENT_FLAGS_MODIFIERS{
    std::make_pair(static_cast<uint64_t>(NX_COMMANDMASK), Modifier::Command),
    std::make_pair(static_cast<uint64_t>(NX_DEVICERCMDKEYMASK), Modifier::RightCommand),
    std::make_pair(static_cast<uint64_t>(NX_DEVICELCMDKEYMASK), Modifier::LeftCommand),

    std::make_pair(static_cast<uint64_t>(NX_SHIFTMASK), Modifier::Shift),
    std::make_pair(static_cast<uint64_t>(NX_DEVICELSHIFTKEYMASK), Modifier::LeftShift),
    std::make_pair(static_cast<uint64_t>(NX_DEVICERSHIFTKEYMASK), Modifier::RightShift),

    std::make_pair(static_cast<uint64_t>(NX_ALTERNATEMASK), Modifier::Option),
    std::make_pair(static_cast<uint64_t>(NX_DEVICELALTKEYMASK), Modifier::LeftOption),
    std::make_pair(static_cast<uint64_t>(NX_DEVICERALTKEYMASK), Modifier::RightOption),

    std::make_pair(static_cast<uint64_t>(NX_CONTROLMASK), Modifier::Control),
    std::make_pair(static_cast<uint64_t>(NX_DEVICELCTLKEYMASK), Modifier::LeftControl),
    std::make_pair(static_cast<uint64_t>(NX_DEVICERCTLKEYMASK), Modifier::RightControl),
};

constexpr std::array SPECIAL_CHARACTERS_TO_KEYCODES{
    std::make_pair(U'\n', kVK_Return),
};

class AutoType::AutoTypeImpl {
  private:
    auto_release<CGEventSourceRef> event_source_ =
        CGEventSourceCreate(kCGEventSourceStateHIDSystemState);

    static constexpr int MAX_KEYBOARD_LAYOUT_CHAR_CODE = 127;
    std::unordered_map<char32_t, KeyCodeWithModifiers> keyboard_layout_ = {};
    CFDataRef keyboard_layout_data_ = nullptr;

  public:
    AutoTypeResult key_move(Direction direction, char32_t character, os_key_code_t code,
                            CGEventFlags flags) {
        auto down = direction == Direction::Down;
        auto_release event = CGEventCreateKeyboardEvent(event_source_, code, down);
        if (character) {
            if (!set_event_char(event, character)) {
                return throw_or_return(AutoTypeResult::BadArg,
                                       std::string("Bad character: ") + std::to_string(character));
            }
        }
        if (flags) {
            CGEventSetFlags(event, flags);
        }
        CGEventPost(kCGHIDEventTap, event);

        auto start_time = std::chrono::system_clock::now();
        auto key_check_code = code;
        for (auto [original_code, check_code] : KEY_CHECK_CODES) {
            if (original_code == code) {
                key_check_code = check_code;
            }
        }
        while (true) {
            auto key_state =
                CGEventSourceKeyState(kCGEventSourceStateHIDSystemState, key_check_code);
            if (key_state == down) {
                break;
            }
            auto elapsed = std::chrono::system_clock::now() - start_time;
            auto wait_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
            if (wait_ms.count() > KEY_PRESS_TOTAL_WAIT_TIME_MS) {
                return throw_or_return(AutoTypeResult::KeyPressFailed,
                                       std::string("Key state didn't change: key ") +
                                           std::to_string(code) + " should be " +
                                           (down ? "down" : "up"));
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(KEY_PRESS_LOOP_WAIT_TIME_MS));
        }

        return AutoTypeResult::Ok;
    }

    static bool set_event_char(CGEventRef event, char32_t character) {
        auto_release cfstr =
            CFStringCreateWithBytes(kCFAllocatorDefault, reinterpret_cast<UInt8 *>(&character),
                                    sizeof(character), kCFStringEncodingUTF32LE, false);
        if (!cfstr) {
            return false;
        }
        auto length = CFStringGetLength(cfstr);
        std::vector<UniChar> utf16_data(length);
        CFStringGetCharacters(cfstr, CFRangeMake(0, length), utf16_data.data());
        CGEventKeyboardSetUnicodeString(event, utf16_data.size(), utf16_data.data());
        return true;
    }

    void read_keyboard_layout() {
        auto_release keyboard = TISCopyCurrentKeyboardInputSource();
        const auto *layout_data = static_cast<CFDataRef>(
            TISGetInputSourceProperty(keyboard, kTISPropertyUnicodeKeyLayoutData));

        if (layout_data == keyboard_layout_data_) {
            return;
        }

        const auto *layout =
            reinterpret_cast<const UCKeyboardLayout *>(CFDataGetBytePtr(layout_data));
        auto kbd_type = LMGetKbdType();

        uint32_t keys_down = 0;
        std::array<UniChar, 4> chars{};
        UniCharCount length = 0;

        keyboard_layout_.clear();

        static constexpr auto SHIFT_MASK = (static_cast<UInt32>(shiftKey) >> 8U) & 0xFFU;
        static constexpr std::array MODIFIER_STATES{
            std::make_pair(0U, Modifier::None), std::make_pair(SHIFT_MASK, Modifier::Shift),
            // other modifiers cause issues in Terminal and similar
        };
        for (int code = 0; code <= MAX_KEYBOARD_LAYOUT_CHAR_CODE; code++) {
            for (auto [mod_state, modifier] : MODIFIER_STATES) {
                auto status = UCKeyTranslate(layout, code, kUCKeyActionDown, mod_state, kbd_type,
                                             kUCKeyTranslateNoDeadKeysBit, &keys_down, 4, &length,
                                             chars.data());
                auto ch = chars[0];
                if (status == noErr && length == 1 && ch && !keyboard_layout_.count(ch)) {
                    KeyCodeWithModifiers code_with_modifier{};
                    code_with_modifier.code = code;
                    code_with_modifier.modifier = modifier;
                    keyboard_layout_.emplace(ch, code_with_modifier);
                }
            }
        }

        for (auto [character, code] : SPECIAL_CHARACTERS_TO_KEYCODES) {
            if (!keyboard_layout_.count(character)) {
                KeyCodeWithModifiers kc{};
                kc.code = code;
                keyboard_layout_.emplace(character, kc);
            }
        }

        keyboard_layout_data_ = layout_data;
    }

    std::optional<KeyCodeWithModifiers> char_to_key_code(char32_t character) {
        auto code = keyboard_layout_.find(character);
        if (code == keyboard_layout_.end()) {
            return std::nullopt;
        }
        return code->second;
    }

    static CGEventFlags modifier_to_flags(Modifier modifier) {
        if (modifier == Modifier::None) {
            return 0;
        }

        CGEventFlags flags = 0;
        for (auto [flag, mod_check] : EVENT_FLAGS_MODIFIERS) {
            if ((modifier & mod_check) == mod_check) {
                flags |= flag;
            }
        }
        return flags;
    }

    static void handle_pending_events() {
        while (CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, true) == kCFRunLoopRunHandledSource) {
            // handle all pending events
            // otherwise native_frontmost_app doesn't return correct results
        }
    }
};

AutoType::AutoType() : impl_(std::make_unique<AutoType::AutoTypeImpl>()) {}

AutoType::~AutoType() = default;

AutoTypeResult AutoType::key_move(Direction direction, char32_t character,
                                  std::optional<os_key_code_t> code, Modifier modifier) {
    auto flags = AutoTypeImpl::modifier_to_flags(modifier);
    return impl_->key_move(direction, character, code.value_or(0), flags);
}

Modifier AutoType::get_pressed_modifiers() {
    auto flags = CGEventSourceFlagsState(kCGEventSourceStateHIDSystemState);
    auto pressed_modifiers = Modifier::None;
    for (auto [flag, modifier] : EVENT_FLAGS_MODIFIERS) {
        if (flags & flag) {
            pressed_modifiers = pressed_modifiers | modifier;
        }
    }
    return pressed_modifiers;
}

Modifier AutoType::shortcut_modifier() { return Modifier::Command; }

std::optional<os_key_code_t> AutoType::os_key_code(KeyCode code) {
    if (code == KeyCode::Undefined) {
        return std::nullopt;
    }
    auto native_key_code = map_key_code(code);
    if (!native_key_code && code != KeyCode::A) { // A maps to kVK_ANSI_A = 0
        return std::nullopt;
    }
    return native_key_code;
}

std::optional<KeyCodeWithModifiers> AutoType::os_key_code_for_char(char32_t character) {
    impl_->read_keyboard_layout();
    return impl_->char_to_key_code(character);
}

std::vector<std::optional<KeyCodeWithModifiers>>
AutoType::os_key_codes_for_chars(std::u32string_view text) {
    impl_->read_keyboard_layout();
    std::vector<std::optional<KeyCodeWithModifiers>> result(text.length());
    auto length = text.length();
    for (size_t i = 0; i < length; i++) {
        result[i] = impl_->char_to_key_code(text[i]);
    }
    return result;
}

pid_t AutoType::active_pid() {
    impl_->handle_pending_events();
    return native_frontmost_app_pid();
}

AppWindow AutoType::active_window(ActiveWindowArgs args) {
    impl_->handle_pending_events();
    auto app = native_frontmost_app();
    if (!app.pid) {
        return {};
    }

    AppWindow result = {};
    result.pid = app.pid;
    result.app_name = app.name;

    auto_release windows = CGWindowListCopyWindowInfo(
        kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements, kCGNullWindowID);
    if (windows) {
        auto count = CFArrayGetCount(windows);
        for (auto i = 0; i < count; i++) {
            const auto *window =
                reinterpret_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(windows, i));
            if (get_number_from_dictionary(window, kCGWindowLayer) != 0) {
                continue;
            }
            if (get_number_from_dictionary(window, kCGWindowOwnerPID) != app.pid) {
                continue;
            }
            // std::cout << cfstring_to_string(CFCopyDescription(window)) << std::endl;

            result.window_id = get_number_from_dictionary(window, kCGWindowNumber);
            if (args.get_window_title) {
                result.title = get_string_from_dictionary(window, kCGWindowName);
            }
            if (result.app_name.empty()) {
                result.app_name = get_string_from_dictionary(window, kCGWindowOwnerName);
            }
            break;
        }
    }

    if (args.get_window_title) {
        result.title = ax_get_focused_window_title(app.pid);
    }

    if (args.get_browser_url && !app.bundle_id.empty()) {
        bool is_browser = std::find(BROWSER_APP_BUNDLE_IDS.begin(), BROWSER_APP_BUNDLE_IDS.end(),
                                    app.bundle_id) != BROWSER_APP_BUNDLE_IDS.end();
        if (is_browser) {
            auto native_win_info = native_window_info(app.pid);
            if (args.get_window_title && result.title.empty() && !native_win_info.title.empty()) {
                result.title = native_win_info.title;
            }
            if (!native_win_info.url.empty()) {
                result.url = native_win_info.url;
            }
        }
    }

    return result;
}

bool AutoType::show_window(const AppWindow &window) {
    if (!window.pid) {
        return false;
    }
    impl_->handle_pending_events();
    return native_show_app(window.pid);
}

AutoTypeTextTransaction AutoType::begin_batch_text_entry() { return AutoTypeTextTransaction(); }

} // namespace keyboard_auto_type
