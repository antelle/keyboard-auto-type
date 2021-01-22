#pragma once

#include <string>
#include <string_view>

namespace keyboard_auto_type_test {

void wait_millis(long ms);
void launch_text_editor(std::string_view file_name);
void kill_text_editor();
bool is_text_editor_app_name(std::string_view app_name);

} // namespace keyboard_auto_type_test