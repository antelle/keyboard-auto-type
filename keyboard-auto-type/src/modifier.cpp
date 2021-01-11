#include "keyboard-auto-type.h"

namespace keyboard_auto_type {

Modifier operator+(Modifier lhs, Modifier rhs) {
    return static_cast<Modifier>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}

Modifier operator|(Modifier lhs, Modifier rhs) {
    return static_cast<Modifier>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}

Modifier operator&(Modifier lhs, Modifier rhs) {
    return static_cast<Modifier>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
}

} // namespace keyboard_auto_type
