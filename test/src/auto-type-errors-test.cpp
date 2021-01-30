#include <string>

#include "gtest/gtest.h"
#include "keyboard-auto-type.h"

namespace kbd = keyboard_auto_type;

namespace keyboard_auto_type_test {

class AutoTypeErrorsTest : public testing::Test {
  protected:
    virtual void TearDown() {
        kbd::AutoType typer;
        typer.key_move(kbd::Direction::Up, kbd::Modifier::Shift);
    }
};

#if __cpp_exceptions && !defined(KEYBOARD_AUTO_TYPE_NO_EXCEPTIONS)
#define ASSERT_THROWS_OR_RETURNS(statement, expected_exception, expected_result)                   \
    ASSERT_THROW(statement, expected_exception)
#else
#define ASSERT_THROWS_OR_RETURNS(statement, expected_exception, expected_result)                   \
    ASSERT_EQ(expected_result, statement)
#endif

TEST_F(AutoTypeErrorsTest, key_press_undefined) {
    kbd::AutoType typer;
    ASSERT_THROWS_OR_RETURNS(typer.key_press(kbd::KeyCode::Undefined), std::invalid_argument,
                             kbd::AutoTypeResult::BadArg);
}

TEST_F(AutoTypeErrorsTest, key_press_bad_key) {
    kbd::AutoType typer;
    ASSERT_THROWS_OR_RETURNS(typer.key_press(kbd::KeyCode::KeyCodeCount), std::invalid_argument,
                             kbd::AutoTypeResult::BadArg);
}

TEST_F(AutoTypeErrorsTest, text_null_char) {
    kbd::AutoType typer;
    std::u32string str{U'\0'};
    ASSERT_THROWS_OR_RETURNS(typer.text(str), std::invalid_argument, kbd::AutoTypeResult::BadArg);
}

TEST_F(AutoTypeErrorsTest, text_modifier_not_released) {
    kbd::AutoType typer;
    typer.set_auto_unpress_modifiers(false);
    typer.set_unpress_modifiers_total_wait_time(std::chrono::milliseconds(0));
    typer.key_move(kbd::Direction::Down, kbd::Modifier::Shift);
    ASSERT_THROWS_OR_RETURNS(typer.text(U"a"), std::runtime_error,
                             kbd::AutoTypeResult::ModifierNotReleased);
}

TEST_F(AutoTypeErrorsTest, key_move_bad_key) {
    kbd::AutoType typer;
    ASSERT_THROWS_OR_RETURNS(typer.key_move(kbd::Direction::Down, kbd::KeyCode::KeyCodeCount),
                             std::invalid_argument, kbd::AutoTypeResult::BadArg);
}

TEST_F(AutoTypeErrorsTest, key_move_bad_utf_surrogate) {
    kbd::AutoType typer;
    ASSERT_THROWS_OR_RETURNS(typer.key_move(kbd::Direction::Down, 0xD801, std::nullopt),
                             std::invalid_argument, kbd::AutoTypeResult::BadArg);
}

} // namespace keyboard_auto_type_test
