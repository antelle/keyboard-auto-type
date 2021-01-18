#include "platform-util.h"

#include <Carbon/Carbon.h>

namespace keyboard_auto_type_test {

void wait_millis(long ms) { CFRunLoopRunInMode(kCFRunLoopDefaultMode, ms / 1000., false); }

void launch_text_editor(std::string_view file_name) {
    system(("open /System/Applications/TextEdit.app " + std::string(file_name)).c_str());
}

void kill_text_editor() { system("killall TextEdit >/dev/null 2>/dev/null"); }

bool is_text_editor_app_name(std::string_view app_name) { return app_name == "TextEdit"; }

} // namespace keyboard_auto_type_test