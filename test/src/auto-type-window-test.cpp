#include <filesystem>
#include <fstream>
#include <string>

#include "gtest/gtest.h"
#include "keyboard-auto-type.h"
#include "platform-util.h"

namespace kbd = keyboard_auto_type;

namespace keyboard_auto_type_test {

class AutoTypeWindowTest : public testing::Test {
  protected:
    static constexpr std::string_view file_name = "build/test.txt";

    static void SetUpTestSuite() { create_file(); }

    virtual void SetUp() {
        kill_text_editor();
        wait_millis(100);
    }

    virtual void TearDown() { kill_text_editor(); }

    static void create_file() {
        std::filesystem::remove(file_name);
        std::fstream(std::string(file_name), std::ios::out).close();
    }
};

TEST_F(AutoTypeWindowTest, active_pid) {
    kbd::AutoType typer;

    auto pid = typer.active_pid();
    ASSERT_NE(0, pid);

    launch_text_editor(file_name);
    for (auto i = 0; i < 100; i++) {
        wait_millis(100);
        auto active_pid = typer.active_pid();
        if (pid != active_pid) {
            return;
        }
    }

    FAIL() << "active_pid didn't change";
}

TEST_F(AutoTypeWindowTest, active_window) {
    kbd::AutoType typer;

    auto pid = typer.active_pid();

    launch_text_editor(file_name);
    for (auto i = 0; i < 100; i++) {
        wait_millis(100);
        auto active_pid = typer.active_pid();
        if (pid != active_pid) {
            wait_millis(1000);

            auto win = typer.active_window();
            ASSERT_EQ(active_pid, win.pid);
            ASSERT_TRUE(win.window_id);
            ASSERT_FALSE(win.app_name.empty());
            ASSERT_TRUE(win.title.empty());
            ASSERT_TRUE(win.url.empty());

            win = typer.active_window({.get_window_title = true, .get_browser_url = true});
            ASSERT_EQ(active_pid, win.pid);
            ASSERT_TRUE(win.window_id);
            ASSERT_FALSE(win.app_name.empty());
            ASSERT_FALSE(win.title.empty());
            ASSERT_TRUE(win.url.empty());

            return;
        }
    }

    FAIL() << "Text editor didn't start";
}

TEST_F(AutoTypeWindowTest, show_window) {
    kbd::AutoType typer;

    auto self_window = typer.active_window();
    ASSERT_EQ(self_window.pid, typer.active_pid());
    ASSERT_NE(0, self_window.pid);

    launch_text_editor(file_name);
    for (auto i = 0; i < 100; i++) {
        wait_millis(100);
        auto editor_window = typer.active_window();
        ASSERT_EQ(editor_window.pid, typer.active_pid());
        if (editor_window.pid != self_window.pid) {
            wait_millis(1000);

            auto win = typer.active_window();
            ASSERT_EQ(win.pid, typer.active_pid());
            ASSERT_EQ(editor_window.pid, win.pid);

            auto shown = typer.show_window(self_window);
            ASSERT_TRUE(shown);
            wait_millis(1000);

            win = typer.active_window();
            ASSERT_EQ(win.pid, typer.active_pid());
            ASSERT_EQ(self_window.pid, win.pid);

            shown = typer.show_window(editor_window);
            ASSERT_TRUE(shown);
            wait_millis(1000);

            win = typer.active_window();
            ASSERT_EQ(win.pid, typer.active_pid());
            ASSERT_EQ(editor_window.pid, win.pid);

            return;
        }
    }

    FAIL() << "Text editor didn't start";
}

} // namespace keyboard_auto_type_test