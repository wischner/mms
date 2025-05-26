#include <mms/mms.h>

#include <gtest/gtest.h>

using mms::postrack;

TEST(Postrack, InitialState)
{
    postrack p;
    EXPECT_EQ(p.line(), 1);
    EXPECT_EQ(p.column(), 1);
}

TEST(Postrack, UpdatePositionBasic)
{
    postrack p;
    p.update_position('a');  // Line 1, Col 2
    p.update_position('b');  // Line 1, Col 3
    p.update_position('\n'); // Line 2, Col 1
    p.update_position('c');  // Line 2, Col 2

    EXPECT_EQ(p.line(), 2);
    EXPECT_EQ(p.column(), 2);

    const auto &newlines = p.newline_positions();
    ASSERT_EQ(newlines.size(), 1);
    EXPECT_EQ(*newlines.begin(), 2); // '\n' was at position 2
}

TEST(Postrack, PutbackSingleChar)
{
    postrack p;
    // col 1
    p.update_position('x');  // pos 0, col 2
    p.update_position('y');  // pos 1, col 3
    p.update_position('\n'); // pos 2, newline, col 1
    p.update_position('z');  // pos 3, col 2

    p.adjust_position_on_putback('z'); // move back from pos 3 → pos 2
    EXPECT_EQ(p.line(), 2);
    EXPECT_EQ(p.column(), 1); // back to beginning of line 2

    p.adjust_position_on_putback('\n'); // move back from pos 2 → pos 1
    EXPECT_EQ(p.line(), 1);
    EXPECT_EQ(p.column(), 3); // back to char 'y'
}

TEST(Postrack, AddAndUseBookmark)
{
    postrack p;

    // Simulate: ab\ncd\n (positions: 0 1 2 3 4 5)
    p.update_position('a');  // pos 0
    p.update_position('b');  // pos 1
    p.update_position('\n'); // pos 2 → line 2
    p.update_position('c');  // pos 3 → line 2, col 1
    p.add_bookmark();        // 🔧 Add bookmark exactly when at pos 3
    p.update_position('d');  // pos 4
    p.update_position('\n'); // pos 5

    p.set_position(3); // Return to bookmark at pos 3
    EXPECT_EQ(p.line(), 2);
    EXPECT_EQ(p.column(), 1); // ✅ 'c' is first char on line 2
}

TEST(Postrack, ColumnComputationAroundNewlines)
{
    postrack p;
    p.update_position('a');  // 0 → line 1 col 2
    p.update_position('\n'); // 1 → line 2 col 1
    p.update_position('b');  // 2 → line 2 col 2
    p.update_position('c');  // 3 → line 2 col 3

    EXPECT_EQ(p.line(), 2);
    EXPECT_EQ(p.column(), 3);

    p.set_position(3); // Back to 'c'
    EXPECT_EQ(p.line(), 2);
    EXPECT_EQ(p.column(), 2);
}

TEST(Postrack, SetPositionWithoutBookmark)
{
    postrack p;

    // Simulate: abc\ndef\n (positions: 0 1 2 3 4 5 6 7)
    const char *text = "abc\ndef\n";
    for (const char *c = text; *c; ++c)
        p.update_position(*c);

    p.set_position(7); // Position at '\n'
    EXPECT_EQ(p.line(), 2);
    EXPECT_EQ(p.column(), 4);
}

TEST(Postrack, MultipleConsecutiveNewlines)
{
    postrack p;
    p.update_position('\n'); // pos 0 → line 2
    p.update_position('\n'); // pos 1 → line 3
    p.update_position('x');  // pos 2 → line 3, col 2

    EXPECT_EQ(p.line(), 3);
    EXPECT_EQ(p.column(), 2);

    const auto &newlines = p.newline_positions();
    ASSERT_EQ(newlines.size(), 2);
    EXPECT_TRUE(newlines.count(0));
    EXPECT_TRUE(newlines.count(1));
}

TEST(Postrack, EmptyLineBetweenText)
{
    postrack p;
    p.update_position('a');  // 0
    p.update_position('\n'); // 1
    p.update_position('\n'); // 2
    p.update_position('b');  // 3

    EXPECT_EQ(p.line(), 3);
    EXPECT_EQ(p.column(), 2);
}

TEST(Postrack, PutbackOverMultipleLines)
{
    postrack p;
    const char *text = "a\nb\nc";
    for (const char *c = text; *c; ++c)
        p.update_position(*c);

    EXPECT_EQ(p.line(), 3);
    EXPECT_EQ(p.column(), 2); // after 'c'

    p.adjust_position_on_putback('c');
    EXPECT_EQ(p.line(), 3);
    EXPECT_EQ(p.column(), 1);

    p.adjust_position_on_putback('\n'); // back to line 2
    EXPECT_EQ(p.line(), 2);
    EXPECT_EQ(p.column(), 2);

    p.adjust_position_on_putback('b');
    EXPECT_EQ(p.line(), 2);
    EXPECT_EQ(p.column(), 1);
}

TEST(Postrack, ResetToBeginning)
{
    postrack p;
    p.update_position('a');
    p.update_position('\n');
    p.update_position('b');
    p.set_position(0); // go back to start

    EXPECT_EQ(p.line(), 1);
    EXPECT_EQ(p.column(), 1);
}

TEST(Postrack, SeekToMiddleOfLine)
{
    postrack p;
    const char *text = "abc\ndef";
    for (const char *c = text; *c; ++c)
        p.update_position(*c);

    p.set_position(5); // pointing to 'e'
    EXPECT_EQ(p.line(), 2);
    EXPECT_EQ(p.column(), 2);
}
