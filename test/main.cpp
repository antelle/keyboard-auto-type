#if __APPLE__
#include <Carbon/Carbon.h>
#include <unistd.h>
#endif

#include <array>
#include <chrono>
#include <codecvt>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#include "gtest/gtest.h"
#include "keyboard-auto-type.h"

namespace kbd = keyboard_auto_type;

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

class AutoTypeTest : public testing::Test {
  protected:
    const std::string file_name = "build/test/test.txt";
    std::filesystem::file_time_type file_mod_time;
    std::u32string expected_text;

  protected:
    // cppcheck-suppress unusedFunction
    static void SetUpTestSuite() {}

    // cppcheck-suppress unusedFunction
    static void TearDownTestSuite() { kill_text_editor(); }

    // cppcheck-suppress unusedFunction
    virtual void SetUp() {
        expected_text = U"";
        open_text_editor();
    }

    // cppcheck-suppress unusedFunction
    virtual void TearDown() {
        save_text();
        wait_for_file_save();
        kill_text_editor();
        check_expected_text();
    }

  private:
    void open_text_editor() {
        create_file();
        kill_text_editor();
        launch_text_editor();
        wait_text_editor_window();
    }

    void launch_text_editor() {
#if __APPLE__
        system(("open /System/Applications/TextEdit.app " + file_name).c_str());
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        system(("start notepad.exe " + file_name).c_str());
#else
        FAIL() << "launch_text_editor is not implemented";
#endif
    }

    void wait_millis(long ms) {
#if __APPLE__
        CFRunLoopRunInMode(kCFRunLoopDefaultMode, ms / 1000., false);
#else
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
#endif
    }

    bool is_text_editor_app_name(std::string_view app_name) {
#if __APPLE__
        return app_name == "TextEdit";
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        return app_name.ends_with("notepad.exe");
#else
        FAIL() << "is_text_editor_app_name is not implemented";
#endif
    }

    void wait_text_editor_window() {
        kbd::AutoType typer;
        for (auto i = 0; i < 100; i++) {
            wait_millis(100);
            auto active_window = typer.active_window();
            if (is_text_editor_app_name(active_window.app_name)) {
                wait_millis(500);
                return;
            }
        }
        FAIL() << "Text editor didn't appear";
    }

    void create_file() {
        std::filesystem::remove(file_name);
        std::fstream fstream(file_name, std::ios::out);
        // fstream << "\xEF\xBB\xBF";
        fstream.close();

        file_mod_time = std::filesystem::last_write_time(file_name);
    }

    void save_text() {
        kbd::AutoType typer;
        wait_millis(10);
        auto active_window = typer.active_window();
        if (is_text_editor_app_name(active_window.app_name)) {
            typer.key_press(0, kbd::KeyCode::S, typer.shortcut_modifier());
            typer.key_press(0, kbd::KeyCode::Q, typer.shortcut_modifier());
        } else {
            FAIL() << "Active app is not a text editor, failed to save";
        }
    }

    static void kill_text_editor() {
#if __APPLE__
        system("killall TextEdit >/dev/null 2>/dev/null");
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        system("taskkill /IM notepad.exe /F >nul 2>nul");
#else
        FAIL() << "kill_text_editor is not implemented";
#endif
    }

    void wait_for_file_save() {
        for (auto i = 0; i < 100; i++) {
            auto last_mod_time = std::filesystem::last_write_time(file_name);
            wait_millis(100);
            if (last_mod_time > file_mod_time) {
                return;
            }
        }
        FAIL() << "File date didn't change";
    }

    void check_expected_text() {
        std::ifstream ifs(file_name);
        std::string data((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
        std::u32string actual_text = converter.from_bytes(data);

        ASSERT_EQ(expected_text, actual_text);
    }

    void open_edit_menu() {
        kbd::AutoType typer;
#if __APPLE__
        // highlight the "Apple" menu
        typer.key_press(0, kbd::KeyCode::F2, kbd::Modifier::Ctrl);
        // go to "Edit" menu
        typer.key_press(0, kbd::KeyCode::RightArrow);
        typer.key_press(0, kbd::KeyCode::RightArrow);
        typer.key_press(0, kbd::KeyCode::RightArrow);
        // open the menu
        typer.key_press(0, kbd::KeyCode::DownArrow);
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        // highlight the first menu ("File")
        typer.key_press(0, kbd::KeyCode::Alt);
        // go to "Edit" menu
        typer.key_press(0, kbd::KeyCode::RightArrow);
        // open the menu
        typer.key_press(0, kbd::KeyCode::DownArrow);
#else
        FAIL() << "open_edit_menu not implemented";
#endif
    }

  public:
    void short_delay() { wait_millis(100); }

    void press_menu_select_all() {
        kbd::AutoType typer;
        open_edit_menu();
#if __APPLE__
        typer.key_press(0, kbd::KeyCode::S);
        typer.key_press(0, kbd::KeyCode::Enter);
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        typer.key_press(0, kbd::KeyCode::A);
#endif
    }

    void press_menu_cut() {
        kbd::AutoType typer;
        open_edit_menu();
#if __APPLE__
        typer.key_press(0, kbd::KeyCode::C);
        typer.key_press(0, kbd::KeyCode::U);
        typer.key_press(0, kbd::KeyCode::Enter);
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        typer.key_press(0, kbd::KeyCode::T);
#endif
    }

    void press_menu_paste() {
        kbd::AutoType typer;
        open_edit_menu();
        typer.key_press(0, kbd::KeyCode::P);
#if __APPLE__
        typer.key_press(0, kbd::KeyCode::Enter);
#endif
    }
};

TEST_F(AutoTypeTest, key_press_letter) {
    kbd::AutoType typer;
    typer.key_press(U'a');
    expected_text = U"a";
}

TEST_F(AutoTypeTest, key_press_two_letters) {
    kbd::AutoType typer;
    typer.key_press(U'a');
    typer.key_press(U'b');
    expected_text = U"ab";
}

TEST_F(AutoTypeTest, key_press_two_lines) {
    kbd::AutoType typer;
    typer.key_press(U'a');
    typer.key_press(U'b');
    typer.key_press(U'\n');
    typer.key_press(U'c');
    typer.key_press(U'd');
    expected_text = U"ab\ncd";
}

TEST_F(AutoTypeTest, key_press_capital) {
    kbd::AutoType typer;
    typer.key_press(U'A');
    typer.key_press(U'b');
    typer.key_press(U'C');
    expected_text = U"AbC";
}

TEST_F(AutoTypeTest, key_press_key_code) {
    kbd::AutoType typer;
    typer.key_press(0, kbd::KeyCode::D0);
    typer.key_press(0, kbd::KeyCode::B);
    expected_text = U"0b";
}

TEST_F(AutoTypeTest, key_press_key_code_with_char) {
    kbd::AutoType typer;
    typer.key_press(U'0', kbd::KeyCode::D0);
    typer.key_press(U'b', kbd::KeyCode::B);
    expected_text = U"0b";
}

TEST_F(AutoTypeTest, key_press_key_code_modifier) {
    kbd::AutoType typer;
    typer.key_press(0, kbd::KeyCode::D1);
    typer.key_press(0, kbd::KeyCode::D1, kbd::Modifier::Shift);
    typer.key_press(0, kbd::KeyCode::C);
    typer.key_press(0, kbd::KeyCode::C, kbd::Modifier::Shift);
    expected_text = U"1!cC";
}

TEST_F(AutoTypeTest, key_press_key_code_modifier_with_char) {
    kbd::AutoType typer;
    typer.key_press(U'1', kbd::KeyCode::D1);
    typer.key_press(U'!', kbd::KeyCode::D1, kbd::Modifier::Shift);
    typer.key_press(U'c', kbd::KeyCode::C);
    typer.key_press(U'C', kbd::KeyCode::C, kbd::Modifier::Shift);
    expected_text = U"1!cC";
}

TEST_F(AutoTypeTest, key_press_menu) {
    kbd::AutoType typer;

    typer.text(U"text");
    short_delay();
    press_menu_select_all();
    short_delay();
    press_menu_cut();
    short_delay();
    typer.text(U"more ");
    short_delay();
    press_menu_paste();
    short_delay();

    expected_text = U"more text";
}

TEST_F(AutoTypeTest, key_press_bad_arg) {
    kbd::AutoType typer;
    typer.key_press(U'a');
    ASSERT_THROW(typer.key_press(0), std::invalid_argument);
    expected_text = U"a";
}

TEST_F(AutoTypeTest, text_unicode_basic) {
    kbd::AutoType typer;
    expected_text = U"";

    constexpr std::array char_ranges{
        // basic latin
        std::pair{U' ', U'~'},
        // latin1-supplement
        std::pair{U'¡', U'ÿ'},
        // latin extended
        std::pair{U'Ā', U'ɏ'},
        // IPA extensions
        std::pair{U'ɐ', U'ʯ'},
        // greek and coptic
        std::pair{U'Ͱ', U'ͳ'},
        std::pair{U'Α', U'Θ'},
        std::pair{U'α', U'μ'},
        std::pair{U'Ϯ', U'ϱ'},
        std::pair{U'ϼ', U'Ͽ'},
        // cyrillic
        std::pair{U'Ѐ', U'Б'},
        // armenian
        std::pair{U'Ա', U'Ե'},
        // hebrew
        std::pair{U'א', U'ה'},
        // arabic
        std::pair{U'ب', U'ج'},
        // bengali
        std::pair{U'৪', U'৭'},
        // thai
        std::pair{U'ก', U'ฅ'},
        // latin extended additional
        std::pair{U'Ḁ', U'ḅ'},
        // arrows
        std::pair{U'←', U'↔'},
        // CJK
        std::pair{U'一', U'三'},
        // hiragana
        std::pair{U'ぁ', U'う'},
        // katakana
        std::pair{U'゠', U'イ'},
    };

    for (auto range : char_ranges) {
        for (auto ch = range.first; ch <= range.second; ch++) {
            expected_text += ch;
        }
        expected_text += U"\n";
    }

    typer.text(expected_text);
}

#if __APPLE__

TEST_F(AutoTypeTest, text_unicode_emoji) {
    expected_text = U"🍆🍑😈";
    kbd::AutoType typer;
    typer.text(expected_text);
}

TEST_F(AutoTypeTest, text_unicode_supplementary_ideographic) {
    expected_text = U"𠀧𠀪";
    kbd::AutoType typer;
    typer.text(expected_text);
}

#endif

TEST_F(AutoTypeTest, shortcut_copy_paste) {
    kbd::AutoType typer;

    // type "hello"
    typer.text(U"hello");
    // "hello"

    // select all
    typer.shortcut(kbd::KeyCode::A);
    typer.shortcut(kbd::KeyCode::C);
    // "[hello]"

    // paste at the end
    typer.key_press(0, kbd::KeyCode::RightArrow);
    typer.text(U" ");
    typer.shortcut(kbd::KeyCode::V);
    typer.text(U" ");
    // "hello hello "

    // cut "hell"
    for (int i = 0; i < 2; i++) {
        typer.key_press(0, kbd::KeyCode::LeftArrow);
    }
    for (int i = 0; i < 4; i++) {
        typer.key_press(0, kbd::KeyCode::LeftArrow, kbd::Modifier::Shift);
    }
    typer.shortcut(kbd::KeyCode::X);
    // "hello [hell]o "

    // paste at the end
    typer.key_press(0, kbd::KeyCode::RightArrow);
    typer.key_press(0, kbd::KeyCode::RightArrow);
    typer.key_press(0, kbd::KeyCode::RightArrow);
    typer.shortcut(kbd::KeyCode::V);
    // "hello o hell"

    expected_text = U"hello o hell";
}
