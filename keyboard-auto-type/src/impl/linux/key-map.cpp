#include "key-map.h"

#include <array>

namespace keyboard_auto_type {

constexpr std::array<uint16_t, static_cast<size_t>(KeyCode::KeyCodeCount)> KEY_MAP{
    
};

uint16_t map_key_code(KeyCode code) {
    auto code_byte = static_cast<uint8_t>(code);
    if (code_byte >= KEY_MAP.size()) {
        return 0;
    }
    return KEY_MAP.at(code_byte);
}

} // namespace keyboard_auto_type
