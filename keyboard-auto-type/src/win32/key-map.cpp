#include "key-map.h"

#include <array>

namespace keyboard_auto_type {

// https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes

constexpr std::array<BYTE, static_cast<size_t>(KeyCode::KeyCodeCount)> KEY_MAP{
    0,

    // numbers
    0x30,
    0x31,
    0x32,
    0x33,
    0x34,
    0x35,
    0x36,
    0x37,
    0x38,
    0x39,

    // letters
    0x41,
    0x42,
    0x43,
    0x44,
    0x45,
    0x46,
    0x47,
    0x48,
    0x49,
    0x4A,
    0x4B,
    0x4C,
    0x4D,
    0x4E,
    0x4F,
    0x50,
    0x51,
    0x52,
    0x53,
    0x54,
    0x55,
    0x56,
    0x57,
    0x58,
    0x59,
    0x5A,

    // function
    VK_F1,
    VK_F2,
    VK_F3,
    VK_F4,
    VK_F5,
    VK_F6,
    VK_F7,
    VK_F8,
    VK_F9,
    VK_F10,
    VK_F11,
    VK_F12,
    VK_F13,
    VK_F14,
    VK_F15,
    VK_F16,
    VK_F17,
    VK_F18,
    VK_F19,
    VK_F20,
    VK_F21,
    VK_F22,
    VK_F23,
    VK_F24,

    // keypad numbers
    VK_NUMPAD0,
    VK_NUMPAD1,
    VK_NUMPAD2,
    VK_NUMPAD3,
    VK_NUMPAD4,
    VK_NUMPAD5,
    VK_NUMPAD6,
    VK_NUMPAD7,
    VK_NUMPAD8,
    VK_NUMPAD9,

    // keypad other
    VK_CLEAR,
    VK_DECIMAL,
    VK_DIVIDE,
    VK_RETURN, // KeypadEnter
    0,         // KeypadEquals
    VK_SUBTRACT,
    VK_MULTIPLY,
    VK_ADD,

    // others
    VK_OEM_5, // Backslash
    VK_OEM_COMMA,
    VK_OEM_PLUS, // Equal
    VK_OEM_3,    // Grave
    VK_OEM_4,    // LeftBracket
    VK_OEM_MINUS,
    VK_OEM_PERIOD,
    VK_OEM_7, // Quote
    VK_OEM_6, // RightBracket
    VK_OEM_1, // Semicolon
    VK_OEM_2, // Slash

    // modifiers
    VK_LWIN,
    VK_CONTROL,
    0, // Function
    VK_MENU,
    VK_SHIFT,
    VK_RWIN,
    VK_RCONTROL,
    VK_RMENU,
    VK_RSHIFT,
    VK_CAPITAL,
    VK_NUMLOCK,
    VK_SCROLL,

    // arrows
    VK_DOWN,
    VK_LEFT,
    VK_RIGHT,
    VK_UP,

    // navigation
    VK_END,
    VK_HOME,
    VK_PRIOR,
    VK_NEXT,

    // actions
    VK_APPS, // ContextMenu
    VK_BACK, // Delete
    VK_ESCAPE,
    VK_DELETE, // ForwardDelete
    VK_HELP,
    VK_VOLUME_MUTE,
    VK_SNAPSHOT, // PrintScreen
    VK_RETURN,
    VK_SPACE,
    VK_TAB,
    VK_VOLUME_DOWN,
    VK_VOLUME_UP,
    VK_PAUSE,
    VK_INSERT,
    VK_SLEEP,
};

BYTE map_key_code(KeyCode code) {
    auto code_byte = static_cast<uint8_t>(code);
    if (code_byte >= KEY_MAP.size()) {
        return 0;
    }
    return KEY_MAP.at(code_byte);
}

} // namespace keyboard_auto_type
