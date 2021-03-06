#include "test-util.h"

#include <chrono>
#include <thread>

#include "gtest/gtest.h"
#include "keyboard-auto-type.h"

namespace kbd = keyboard_auto_type;

namespace keyboard_auto_type_test {

void open_test_app() {
    system("node test-keys-app/start.js");
    wait_test_app_window();
}

bool is_test_app_active() {
    kbd::AutoType typer;
    auto active_window = typer.active_window({.get_window_title = true});
    return active_window.title == "Test keys app";
}

void wait_test_app_window() {
    for (auto i = 0; i < 100; i++) {
        wait_millis(100);
        if (is_test_app_active()) {
            wait_millis(100);
            return;
        }
    }
    FAIL() << "Test app didn't appear";
}

void save_text_and_close_test_app() {
    kbd::AutoType typer;
    wait_millis(10);
    if (is_test_app_active()) {
        typer.shortcut(kbd::KeyCode::S);
        wait_millis(500);
    } else {
        FAIL() << "Active app is not a test app, failed to save";
    }
}

void wait_millis(long ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

} // namespace keyboard_auto_type_test
