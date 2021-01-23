#include "key-map.h"

#include <X11/keysym.h>

#include <array>

namespace keyboard_auto_type {

// https://gitlab.freedesktop.org/xorg/proto/xorgproto/-/blob/master/include/X11/keysymdef.h

constexpr std::array<os_key_code_t, static_cast<size_t>(KeyCode::KeyCodeCount)> KEY_MAP{
    0,

    // numbers
    XK_0, XK_1, XK_2, XK_3, XK_4, XK_5, XK_6, XK_7, XK_8, XK_9,

    // letters
    XK_a, XK_b, XK_c, XK_d, XK_e, XK_f, XK_g, XK_h, XK_i, XK_j, XK_k, XK_l, XK_m, XK_n, XK_o, XK_p,
    XK_q, XK_r, XK_s, XK_t, XK_u, XK_v, XK_w, XK_x, XK_y, XK_z,

    // function
    XK_F1, XK_F2, XK_F3, XK_F4, XK_F5, XK_F6, XK_F7, XK_F8, XK_F9, XK_F10, XK_F11, XK_F12, XK_F13,
    XK_F14, XK_F15, XK_F16, XK_F17, XK_F18, XK_F19, XK_F20, XK_F21, XK_F22, XK_F23, XK_F24,

    // keypad numbers
    XK_KP_0, XK_KP_1, XK_KP_2, XK_KP_3, XK_KP_4, XK_KP_5, XK_KP_6, XK_KP_7, XK_KP_8, XK_KP_9,

    // keypad other
    0, // KeypadClear
    XK_KP_Decimal, XK_KP_Divide, XK_KP_Enter, XK_KP_Equal, XK_KP_Subtract, XK_KP_Multiply,
    XK_KP_Add,

    // others
    XK_backslash, XK_comma, XK_equal, XK_grave, XK_braceleft, XK_minus, XK_period, XK_apostrophe,
    XK_braceright, XK_semicolon, XK_slash,

    // modifiers
    XK_Meta_L, // or is it XK_Super_L?
    XK_Control_L,
    0, // Function
    XK_Alt_L, XK_Shift_L, XK_Meta_R, XK_Control_R, XK_Alt_R, XK_Shift_R, XK_Caps_Lock, XK_Num_Lock,
    XK_Scroll_Lock,

    // arrows
    XK_Down, XK_Left, XK_Right, XK_Up,

    // navigation
    XK_End, XK_Home, XK_Page_Down, XK_Page_Up,

    // actions
    XK_Menu, XK_BackSpace, XK_Escape, XK_Delete, XK_Help,
    0, // Mute
    0, // PrintScreen
    XK_Return, XK_space, XK_Tab,
    0,         // VolumeDown
    0,         // VolumeUp
    XK_Cancel, // or is it XK_Break?
    XK_Insert,
    0, // Sleep,
};

os_key_code_t map_key_code(KeyCode code) {
    auto code_byte = static_cast<uint8_t>(code);
    if (code_byte >= KEY_MAP.size()) {
        return 0;
    }
    return KEY_MAP.at(code_byte);
}

} // namespace keyboard_auto_type
