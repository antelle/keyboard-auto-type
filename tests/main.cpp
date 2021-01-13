#include <unistd.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "keyboard-auto-type.h"

namespace kbd = keyboard_auto_type;

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

class AutoTypeTest : public testing::Test {
  protected:
    const std::string file_name = "build/tests/test.txt";
    time_t file_mod_time;
    std::string expected_text;

  protected:
    static void SetUpTestSuite() {}

    static void TearDownTestSuite() { kill_text_editor(); }

    virtual void SetUp() { open_text_editor(); }

    virtual void TearDown() {
        save_text();
        wait_for_file_save();
        kill_text_editor();
        check_expected_text();
    }

  private:
    static void kill_text_editor() {
#if __APPLE__
        system("killall TextEdit >/dev/null 2>/dev/null");
#else
        FAIL() << "kill_text_editor is not implemented";
#endif
    }

    void launch_text_editor() {
#if __APPLE__
        system(("open /System/Applications/TextEdit.app " + file_name).c_str());
#else
        FAIL() << "launch_text_editor is not implemented";
#endif
        sleep(1); // TODO: wait until it's active
    }

    void open_text_editor() {
        create_file();
        kill_text_editor();
        launch_text_editor();
    }

    void create_file() {
        std::filesystem::remove(file_name);
        std::fstream(file_name, std::ios::out).close();

        auto ftime = std::filesystem::last_write_time(file_name);
        file_mod_time = decltype(ftime)::clock::to_time_t(ftime);
    }

    void save_text() {
        kbd::AutoType typer;
        typer.key_press(0, kbd::KeyCode::ANSI_S, typer.shortcut_modifier());
        typer.key_press(0, kbd::KeyCode::ANSI_Q, typer.shortcut_modifier());
    }

    void wait_for_file_save() {
        for (auto i = 0; i < 100; i++) {
            auto ftime = std::filesystem::last_write_time(file_name);
            auto last_mod_time = decltype(ftime)::clock::to_time_t(ftime);
            usleep(100000);
            if (last_mod_time > file_mod_time) {
                return;
            }
        }
        FAIL() << "File date didn't change";
    }

    void check_expected_text() {
        std::ifstream ifs(file_name);
        std::string actual_text((std::istreambuf_iterator<char>(ifs)),
                                std::istreambuf_iterator<char>());

        ASSERT_EQ(expected_text, actual_text);
    }
};

TEST_F(AutoTypeTest, key_press_letter) {
    kbd::AutoType typer;
    typer.key_press(L'a');
    expected_text = "a";
}

TEST_F(AutoTypeTest, key_press_two_letters) {
    kbd::AutoType typer;
    typer.key_press(L'a');
    typer.key_press(L'b');
    expected_text = "ab";
}

TEST_F(AutoTypeTest, key_press_two_lines) {
    kbd::AutoType typer;
    typer.key_press(L'a');
    typer.key_press(L'b');
    typer.key_press(L'\n');
    typer.key_press(L'c');
    typer.key_press(L'd');
    expected_text = "ab\ncd";
}
