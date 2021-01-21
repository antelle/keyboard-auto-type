#include "platform-util.h"
#include <unistd.h>

namespace keyboard_auto_type_test {

void wait_millis(long ms) { usleep(ms * 1000); }

void launch_text_editor(std::string_view file_name) {
    system(("open /usr/bin/gedit " + std::string(file_name)).c_str());
}

void kill_text_editor() { system("killall gedit >/dev/null 2>/dev/null"); }

bool is_text_editor_app_name(std::string_view app_name) { return app_name.ends_with("gedit"); }

} // namespace keyboard_auto_type_test