#include <filesystem>
#include <string>
#include <fstream>

#include <gtest/gtest.h>

#include <mms/mms.h>

namespace fs = std::filesystem;
using mms::bookmark;
using mms::source;

// Provided by main.cpp or test framework setup
extern fs::path exeDir;

// Helper: path to file in bin/data/
static fs::path data_file(const std::string &name)
{
    return exeDir / "data" / name;
}

TEST(Source, ReadsEntirePlainTextFile)
{
    auto path = data_file("test-plain-text.txt");
    source s(path.c_str());

    std::string result;
    while (s)
        result += static_cast<char>(s.get());

    // Should end in correct line
    EXPECT_GE(s.line(), 4); // It has 4 lines
    EXPECT_FALSE(s);        // At EOF
    EXPECT_EQ(result.back(), '.');
    EXPECT_NE(result.find("multiple lines"), std::string::npos);
}

TEST(Source, PeekAndGetReturnSameChar)
{
    auto path = data_file("test-plain-text.txt");
    source s(path.c_str());

    int first_peek = s.peek();
    int first_get = s.get();

    EXPECT_EQ(first_peek, first_get);

    int second = s.get();
    EXPECT_NE(first_get, second);
}

TEST(Source, PutbackRevertsPosition)
{
    auto path = data_file("test-plain-text.txt");
    source s(path.c_str());

    int ch1 = s.get();
    int ch2 = s.get();

    s.putback(); // Should revert ch2
    int ch2_re = s.get();

    EXPECT_EQ(ch2, ch2_re);
}

TEST(Source, TracksLineAndColumnCorrectly)
{
    auto path = data_file("test-plain-text.txt");
    source s(path.c_str());

    // Consume first line
    while (s && s.get() != '\n')
    {
    }

    EXPECT_EQ(s.line(), 2);
    EXPECT_EQ(s.column(), 1);

    s.get(); // Read first char of second line
    EXPECT_EQ(s.line(), 2);
    EXPECT_EQ(s.column(), 2);
}

TEST(Source, BookmarkAndSeekRestoreState)
{
    auto path = data_file("test-plain-text.txt");
    source s(path.c_str());

    // Read a few characters
    s.get();
    s.get();
    s.get();

    auto b = s.mark();

    int after_mark = s.get();
    EXPECT_NE(after_mark, EOF);

    s.seek(b);

    int again = s.get();
    EXPECT_EQ(again, after_mark); // Re-read the same char
}

TEST(Source, ReadsUTF8ContentWithoutCorruption)
{
    auto path = data_file("test-utf8.txt");
    source s(path.c_str());

    std::string result;
    while (s)
        result += static_cast<char>(s.get());

    // UTF-8 content exists (not validating encoding)
    EXPECT_NE(result.find("€uro"), std::string::npos);
    EXPECT_NE(result.find("𐍈"), std::string::npos);
}

TEST(Source, ReachesEOFAndOperatorBoolIsFalse)
{
    auto path = data_file("test-plain-text.txt");
    source s(path.c_str());

    while (s.get() != EOF)
    {
    }

    EXPECT_FALSE(s); // Stream should be exhausted
}

TEST(Source, HandlesEmptyFileGracefully)
{
    // Create a temporary empty file
    auto path = exeDir / "data" / "empty.txt";
    {
        std::ofstream out(path);
        ASSERT_TRUE(out.good());
    }

    source s(path.c_str());
    EXPECT_EQ(s.size(), 0);
    EXPECT_FALSE(s); // Already at EOF
    EXPECT_EQ(s.get(), EOF);
    EXPECT_EQ(s.peek(), EOF);
}

TEST(Source, ExtractStringToken)
{
    auto path = data_file("test-plain-text.txt");
    source s(path.c_str());

    std::string word;
    s >> word;

    EXPECT_EQ(word, "Hello,");
}

TEST(Source, ExtractMultipleTokensChained)
{
    auto path = data_file("test-plain-text.txt");
    source s(path.c_str());

    std::string a, b, c;
    s >> a >> b >> c;

    EXPECT_EQ(a, "Hello,");
    EXPECT_EQ(b, "this");
    EXPECT_EQ(c, "is");
}

TEST(Source, ExtractInteger)
{
    // Create file with numeric content
    auto path = exeDir / "data" / "numeric.txt";
    {
        std::ofstream out(path);
        out << "  -123\n456";
    }

    source s(path.c_str());

    int a, b;
    s >> a >> b;

    EXPECT_EQ(a, -123);
    EXPECT_EQ(b, 456);
}

TEST(Source, ExtractCharSkipsWhitespace)
{
    // File with spaced characters
    auto path = exeDir / "data" / "chars.txt";
    {
        std::ofstream out(path);
        out << "  A B";
    }

    source s(path.c_str());

    char a, b;
    s >> a >> b;

    EXPECT_EQ(a, 'A');
    EXPECT_EQ(b, 'B');
}

TEST(Source, ExtractInvalidIntegerThrows)
{
    // File with invalid number
    auto path = exeDir / "data" / "badnum.txt";
    {
        std::ofstream out(path);
        out << "abc";
    }

    source s(path.c_str());
    int x;
    EXPECT_THROW(s >> x, std::runtime_error);
}

TEST(Source, ExtractCharEOFThrows)
{
    auto path = exeDir / "data" / "emptychar.txt";
    {
        std::ofstream out(path); // empty
    }

    source s(path.c_str());
    char c;
    EXPECT_THROW(s >> c, std::runtime_error);
}

TEST(Source, OperatorExtractsString)
{
    auto path = data_file("test-plain-text.txt");
    source s(path.c_str());

    std::string word;
    s >> word;

    EXPECT_EQ(word, "Hello,");
}

TEST(Source, OperatorExtractsMultipleTypes)
{
    // Create temporary file
    auto path = exeDir / "data" / "mixed.txt";
    std::ofstream out(path);
    out << "123 abc Z";
    out.close();

    source s(path.c_str());

    int num;
    std::string word;
    char ch;

    s >> num >> word >> ch;

    EXPECT_EQ(num, 123);
    EXPECT_EQ(word, "abc");
    EXPECT_EQ(ch, 'Z');
}

TEST(Source, PutbackSingleChar)
{
    auto path = data_file("test-plain-text.txt");
    source s(path.c_str());

    char a, b;
    s >> a >> b;

    s.putback();
    char b2;
    s >> b2;

    EXPECT_EQ(b, b2);
}

TEST(Source, MultiplePutbacksRewindCorrectly)
{
    auto path = data_file("test-plain-text.txt");
    source s(path.c_str());

    std::string word;
    s >> word; // Reads "Hello,"

    // Rewind character by character
    for (std::size_t i = 0; i < word.size(); ++i)
        s.putback();

    std::string again;
    s >> again;

    EXPECT_EQ(again, word);
}

TEST(Source, BookmarkRestoresPosition)
{
    auto path = data_file("test-plain-text.txt");
    source s(path.c_str());

    std::string a, b;
    s >> a;

    bookmark bmark = s.mark();
    s >> b;

    EXPECT_FALSE(a.empty());
    EXPECT_FALSE(b.empty());

    s.seek(bmark);

    std::string b2;
    s >> b2;

    EXPECT_EQ(b, b2);
}

TEST(Source, PutbackRebuildsOriginalToken)
{
    auto path = data_file("test-plain-text.txt");
    source s(path.c_str());

    std::string word;
    s >> word;

    // Reinsert in reverse
    for (auto it = word.rbegin(); it != word.rend(); ++it)
        s.putback();

    std::string again;
    s >> again;

    EXPECT_EQ(again, word);
}

TEST(Source, MultiplePutbacksAndBookmarkCombined)
{
    // File with numbers
    auto path = exeDir / "data" / "numbers.txt";
    std::ofstream out(path);
    out << "42 73";
    out.close();

    source s(path.c_str());

    int a;
    s >> a;
    bookmark b = s.mark();

    int b_val;
    s >> b_val;

    s.seek(b);
    int b_val2;
    s >> b_val2;

    EXPECT_EQ(b_val, 73);
    EXPECT_EQ(b_val2, 73);
}