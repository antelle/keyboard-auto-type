#include "../platform-util.h"

namespace keyboard_auto_type_test {

void wait_millis(long ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

void launch_text_editor(std::string_view file_name) {
    system(("start notepad.exe " + std::string(file_name)).c_str());
}

void kill_text_editor() { system("taskkill /IM notepad.exe /F >nul 2>nul"); }

bool is_text_editor_app_name(std::string_view app_name) {
    return app_name.ends_with("notepad.exe");
}

} // namespace keyboard_auto_type_test