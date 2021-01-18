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
#include "platform-util.h"

namespace kbd = keyboard_auto_type;

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

namespace keyboard_auto_type_test {

class AutoTypeKeysTest : public testing::Test {
  protected:
    const std::string file_name = "build/test/test.txt";
    std::filesystem::file_time_type file_mod_time;
    std::u32string expected_text;

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
        wait_millis(100);
        launch_text_editor(file_name);
        wait_text_editor_window();
    }

    void wait_text_editor_window() {
        kbd::AutoType typer;
        for (auto i = 0; i < 100; i++) {
            wait_millis(100);
            auto active_window = typer.active_window();
            if (is_text_editor_app_name(active_window.app_name)) {
                wait_millis(1000);
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
            typer.key_press(kbd::KeyCode::S, typer.shortcut_modifier());
            typer.key_press(kbd::KeyCode::Q, typer.shortcut_modifier());
        } else {
            FAIL() << "Active app is not a text editor, failed to save";
        }
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
        typer.key_press(kbd::KeyCode::F2, kbd::Modifier::Ctrl);
        // go to "Edit" menu
        typer.key_press(kbd::KeyCode::RightArrow);
        typer.key_press(kbd::KeyCode::RightArrow);
        typer.key_press(kbd::KeyCode::RightArrow);
        // open the menu
        typer.key_press(kbd::KeyCode::DownArrow);
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        // highlight the first menu ("File")
        typer.key_press(kbd::KeyCode::Alt);
        // go to "Edit" menu
        typer.key_press(kbd::KeyCode::RightArrow);
        // open the menu
        typer.key_press(kbd::KeyCode::DownArrow);
#else
        FAIL() << "open_edit_menu not implemented";
#endif
    }

  public:
    void press_menu_select_all() {
        kbd::AutoType typer;
        open_edit_menu();
#if __APPLE__
        typer.key_press(kbd::KeyCode::S);
        typer.key_press(kbd::KeyCode::Enter);
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        typer.key_press(kbd::KeyCode::A);
#endif
    }

    void press_menu_cut() {
        kbd::AutoType typer;
        open_edit_menu();
#if __APPLE__
        typer.key_press(kbd::KeyCode::C);
        typer.key_press(kbd::KeyCode::U);
        typer.key_press(kbd::KeyCode::Enter);
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        typer.key_press(kbd::KeyCode::T);
#endif
    }

    void press_menu_paste() {
        kbd::AutoType typer;
        open_edit_menu();
        typer.key_press(kbd::KeyCode::P);
#if __APPLE__
        typer.key_press(kbd::KeyCode::Enter);
#endif
    }
};

TEST_F(AutoTypeKeysTest, text_letter) {
    expected_text = U"a";
    kbd::AutoType typer;
    typer.text(expected_text);
}

TEST_F(AutoTypeKeysTest, text_two_letters) {
    expected_text = U"ab";
    kbd::AutoType typer;
    typer.text(expected_text);
}

TEST_F(AutoTypeKeysTest, text_two_lines) {
    expected_text = U"ab\ncd";
    kbd::AutoType typer;
    typer.text(expected_text);
}

TEST_F(AutoTypeKeysTest, text_capital) {
    expected_text = U"AbC";
    kbd::AutoType typer;
    typer.text(expected_text);
}

TEST_F(AutoTypeKeysTest, text_unicode_basic) {
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

    for (auto [from, to] : char_ranges) {
        auto range_count = 0;
        for (auto ch = from; ch <= to; ch++) {
            expected_text += ch;
            if (++range_count % 50 == 0) {
                expected_text += U"\n";
            }
        }
        expected_text += U"\n";
    }

    typer.text(expected_text);
}

#if __APPLE__
TEST_F(AutoTypeKeysTest, text_unicode_emoji) {
    expected_text = U"🍆🍑😈";
    kbd::AutoType typer;
    typer.text(expected_text);
}
#endif

#if __APPLE__
TEST_F(AutoTypeKeysTest, text_unicode_supplementary_ideographic) {
    expected_text = U"𠀧𠀪";
    kbd::AutoType typer;
    typer.text(expected_text);
}
#endif

TEST_F(AutoTypeKeysTest, key_press_key_code) {
    expected_text = U"0b";
    kbd::AutoType typer;
    typer.key_press(kbd::KeyCode::D0);
    typer.key_press(kbd::KeyCode::B);
}

TEST_F(AutoTypeKeysTest, key_press_key_code_modifier) {
    expected_text = U"1!cC";
    kbd::AutoType typer;
    typer.key_press(kbd::KeyCode::D1);
    typer.key_press(kbd::KeyCode::D1, kbd::Modifier::Shift);
    typer.key_press(kbd::KeyCode::C);
    typer.key_press(kbd::KeyCode::C, kbd::Modifier::Shift);
}

TEST_F(AutoTypeKeysTest, key_press_menu) {
    expected_text = U"more text";
    kbd::AutoType typer;

    typer.text(U"text");
    press_menu_select_all();
    press_menu_cut();
    typer.text(U"more ");
    press_menu_paste();
}

TEST_F(AutoTypeKeysTest, key_press_bad_arg) {
    expected_text = U"";
    kbd::AutoType typer;
    ASSERT_THROW(typer.key_press(kbd::KeyCode::Undefined), std::invalid_argument);
}

TEST_F(AutoTypeKeysTest, shortcut_copy_paste) {
    expected_text = U"hello o hell";
    kbd::AutoType typer;

    // type "hello"
    typer.text(U"hello");
    // "hello"

    // select all
    typer.shortcut(kbd::KeyCode::A);
    typer.shortcut(kbd::KeyCode::C);
    // "[hello]"

    // paste at the end
    typer.key_press(kbd::KeyCode::RightArrow);
    typer.text(U" ");
    typer.shortcut(kbd::KeyCode::V);
    typer.text(U" ");
    // "hello hello "

    // cut "hell"
    for (int i = 0; i < 2; i++) {
        typer.key_press(kbd::KeyCode::LeftArrow);
    }
    for (int i = 0; i < 4; i++) {
        typer.key_press(kbd::KeyCode::LeftArrow, kbd::Modifier::Shift);
    }
    typer.shortcut(kbd::KeyCode::X);
    // "hello [hell]o "

    // paste at the end
    typer.key_press(kbd::KeyCode::RightArrow);
    typer.key_press(kbd::KeyCode::RightArrow);
    typer.key_press(kbd::KeyCode::RightArrow);
    typer.shortcut(kbd::KeyCode::V);
    // "hello o hell"
}

TEST_F(AutoTypeKeysTest, key_move_single) {
    expected_text = U"a";
    kbd::AutoType typer;

    typer.key_move(kbd::Direction::Down, kbd::KeyCode::A);
    typer.key_move(kbd::Direction::Up, kbd::KeyCode::A);
}

TEST_F(AutoTypeKeysTest, key_move_multiple) {
    expected_text = U"aab";
    kbd::AutoType typer;

    typer.key_move(kbd::Direction::Down, kbd::KeyCode::A);
    typer.key_move(kbd::Direction::Up, kbd::KeyCode::A);

    typer.key_move(kbd::Direction::Down, kbd::KeyCode::A);
    typer.key_move(kbd::Direction::Up, kbd::KeyCode::A);

    typer.key_move(kbd::Direction::Down, kbd::KeyCode::B);
    typer.key_move(kbd::Direction::Up, kbd::KeyCode::B);
}

TEST_F(AutoTypeKeysTest, key_move_shift) {
    expected_text = U"aABb";
    kbd::AutoType typer;

    typer.key_move(kbd::Direction::Down, kbd::KeyCode::A);
    typer.key_move(kbd::Direction::Up, kbd::KeyCode::A);

    typer.key_move(kbd::Direction::Down, kbd::KeyCode::Shift);

    typer.key_move(kbd::Direction::Down, kbd::KeyCode::A, kbd::Modifier::Shift);
    typer.key_move(kbd::Direction::Up, kbd::KeyCode::A, kbd::Modifier::Shift);

    typer.key_move(kbd::Direction::Down, kbd::KeyCode::B, kbd::Modifier::Shift);
    typer.key_move(kbd::Direction::Up, kbd::KeyCode::B, kbd::Modifier::Shift);

    typer.key_move(kbd::Direction::Up, kbd::KeyCode::Shift);

    typer.key_move(kbd::Direction::Down, kbd::KeyCode::B);
    typer.key_move(kbd::Direction::Up, kbd::KeyCode::B);
}

TEST_F(AutoTypeKeysTest, key_move_right_shift) {
    expected_text = U"aABb";
    kbd::AutoType typer;

    typer.key_move(kbd::Direction::Down, kbd::KeyCode::A);
    typer.key_move(kbd::Direction::Up, kbd::KeyCode::A);

    typer.key_move(kbd::Direction::Down, kbd::KeyCode::RightShift);

    typer.key_move(kbd::Direction::Down, kbd::KeyCode::A, kbd::Modifier::Shift);
    typer.key_move(kbd::Direction::Up, kbd::KeyCode::A, kbd::Modifier::Shift);

    typer.key_move(kbd::Direction::Down, kbd::KeyCode::B, kbd::Modifier::Shift);
    typer.key_move(kbd::Direction::Up, kbd::KeyCode::B, kbd::Modifier::Shift);

    typer.key_move(kbd::Direction::Up, kbd::KeyCode::RightShift);

    typer.key_move(kbd::Direction::Down, kbd::KeyCode::B);
    typer.key_move(kbd::Direction::Up, kbd::KeyCode::B);
}

TEST_F(AutoTypeKeysTest, key_move_all_keys) {
    expected_text = U"0123456789 abcdefghijklmnopqrstuvwxyz\t0123456789\n/-*+\n\\,=`[-.'];/";
    kbd::AutoType typer;

    static constexpr std::array ALL_KEYS{
        kbd::KeyCode::D0,
        kbd::KeyCode::D1,
        kbd::KeyCode::D2,
        kbd::KeyCode::D3,
        kbd::KeyCode::D4,
        kbd::KeyCode::D5,
        kbd::KeyCode::D6,
        kbd::KeyCode::D7,
        kbd::KeyCode::D8,
        kbd::KeyCode::D9,

        kbd::KeyCode::Space,

        kbd::KeyCode::A,
        kbd::KeyCode::B,
        kbd::KeyCode::C,
        kbd::KeyCode::D,
        kbd::KeyCode::E,
        kbd::KeyCode::F,
        kbd::KeyCode::G,
        kbd::KeyCode::H,
        kbd::KeyCode::I,
        kbd::KeyCode::J,
        kbd::KeyCode::K,
        kbd::KeyCode::L,
        kbd::KeyCode::M,
        kbd::KeyCode::N,
        kbd::KeyCode::O,
        kbd::KeyCode::P,
        kbd::KeyCode::Q,
        kbd::KeyCode::R,
        kbd::KeyCode::S,
        kbd::KeyCode::T,
        kbd::KeyCode::U,
        kbd::KeyCode::V,
        kbd::KeyCode::W,
        kbd::KeyCode::X,
        kbd::KeyCode::Y,
        kbd::KeyCode::Z,

        kbd::KeyCode::Tab,

        kbd::KeyCode::Keypad0,
        kbd::KeyCode::Keypad1,
        kbd::KeyCode::Keypad2,
        kbd::KeyCode::Keypad3,
        kbd::KeyCode::Keypad4,
        kbd::KeyCode::Keypad5,
        kbd::KeyCode::Keypad6,
        kbd::KeyCode::Keypad7,
        kbd::KeyCode::Keypad8,
        kbd::KeyCode::Keypad9,

        kbd::KeyCode::Return,

        kbd::KeyCode::KeypadDivide,
        kbd::KeyCode::KeypadMinus,
        kbd::KeyCode::KeypadMultiply,
        kbd::KeyCode::KeypadPlus,
        kbd::KeyCode::KeypadEnter,

        kbd::KeyCode::Backslash,
        kbd::KeyCode::Comma,
        kbd::KeyCode::Equal,
        kbd::KeyCode::Grave,
        kbd::KeyCode::LeftBracket,
        kbd::KeyCode::Minus,
        kbd::KeyCode::Period,
        kbd::KeyCode::Quote,
        kbd::KeyCode::RightBracket,
        kbd::KeyCode::Semicolon,
        kbd::KeyCode::Slash,

        kbd::KeyCode::UpArrow,
        kbd::KeyCode::DownArrow,
        kbd::KeyCode::LeftArrow,
        kbd::KeyCode::RightArrow,

        kbd::KeyCode::Home,
        kbd::KeyCode::End,
        kbd::KeyCode::PageUp,
        kbd::KeyCode::PageDown,

        kbd::KeyCode::X,
        kbd::KeyCode::BackwardDelete,

        kbd::KeyCode::X,
        kbd::KeyCode::LeftArrow,
        kbd::KeyCode::ForwardDelete,

        kbd::KeyCode::Ctrl,
        kbd::KeyCode::Function,
        kbd::KeyCode::Shift,
        kbd::KeyCode::Option,
        kbd::KeyCode::Meta,
        kbd::KeyCode::Escape,

        kbd::KeyCode::RightControl,
        kbd::KeyCode::RightShift,
        kbd::KeyCode::RightOption,
        kbd::KeyCode::RightMeta,
        kbd::KeyCode::Escape,
    };

    for (auto key_code : ALL_KEYS) {
        auto os_key_code = typer.os_key_code(key_code);
        if (os_key_code.has_value()) {
            typer.key_move(kbd::Direction::Down, key_code);
            typer.key_move(kbd::Direction::Up, key_code);
        }
    }

    wait_millis(100);

    // hide all possible menus
    typer.key_press(kbd::KeyCode::Escape);
    wait_millis(100);
}

class AutoTypeWindowTest : public testing::Test {
  protected:
    static constexpr std::string_view file_name = "build/test/test.txt";

    // cppcheck-suppress unusedFunction
    static void SetUpTestSuite() { create_file(); }

    // cppcheck-suppress unusedFunction
    virtual void SetUp() {
        kill_text_editor();
        wait_millis(100);
    }

    // cppcheck-suppress unusedFunction
    virtual void TearDown() { kill_text_editor(); }

    static void create_file() {
        std::filesystem::remove(file_name);
        std::fstream(file_name, std::ios::out).close();
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
            wait_millis(100);

            win = typer.active_window();
            ASSERT_EQ(win.pid, typer.active_pid());
            ASSERT_EQ(self_window.pid, win.pid);

            shown = typer.show_window(editor_window);
            ASSERT_TRUE(shown);
            wait_millis(100);

            win = typer.active_window();
            ASSERT_EQ(win.pid, typer.active_pid());
            ASSERT_EQ(editor_window.pid, win.pid);

            return;
        }
    }

    FAIL() << "Text editor didn't start";
}

} // namespace keyboard_auto_type_test
