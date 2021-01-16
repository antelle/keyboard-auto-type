#include "carbon-helpers.h"

#include <iostream>
#include <vector>

#include "auto-release.h"

namespace keyboard_auto_type {

std::string cfstring_to_string(CFStringRef str) {
    auto length = CFStringGetLength(str);
    auto max_length = length * 4 + 1;
    std::vector<char> chars(max_length);
    if (CFStringGetCString(str, chars.data(), max_length, kCFStringEncodingUTF8)) {
        return chars.data();
    }
    return "";
}

int get_number_from_dictionary(CFDictionaryRef dictionary, CFStringRef key) {
    CFNumberRef number_ref = nullptr;
    if (!CFDictionaryGetValueIfPresent(dictionary, key,
                                       reinterpret_cast<const void **>(&number_ref))) {
        return 0;
    }
    int value = 0;
    if (!CFNumberGetValue(number_ref, kCFNumberSInt32Type, &value)) {
        return 0;
    }
    return value;
}

std::string get_string_from_dictionary(CFDictionaryRef dictionary, CFStringRef key) {
    CFStringRef str_ref = nullptr;
    if (!CFDictionaryGetValueIfPresent(dictionary, key,
                                       reinterpret_cast<const void **>(&str_ref))) {
        return "";
    }
    return cfstring_to_string(str_ref);
}

std::string ax_get_focused_window_title(pid_t pid) {
    auto_release accessibility_app = AXUIElementCreateApplication(pid);
    auto_release<CFTypeRef> ax_window;
    auto ax_err = AXUIElementCopyAttributeValue(
        accessibility_app,
        kAXFocusedWindowAttribute, // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
        &ax_window);
    if (ax_err != kAXErrorSuccess) {
        return "";
    }

    auto_release<CFTypeRef> ax_title;
    ax_err = AXUIElementCopyAttributeValue(
        reinterpret_cast<AXUIElementRef>(static_cast<CFTypeRef>(ax_window)),
        kAXTitleAttribute, // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
        &ax_title);
    if (ax_err != kAXErrorSuccess) {
        return "";
    }

    return cfstring_to_string(reinterpret_cast<CFStringRef>(static_cast<CFTypeRef>(ax_title)));
}

} // namespace keyboard_auto_type
