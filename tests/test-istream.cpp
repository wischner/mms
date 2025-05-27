#include <gtest/gtest.h>
#include <filesystem>
#include <string>
#include <sstream>
#include <mms/mms.h>

using mms::istream;
namespace fs = std::filesystem;

extern fs::path exeDir;

static fs::path data_file(const std::string &name)
{
    return exeDir / "data" / name;
}

TEST(Istream, BasicLineAndColumnTracking)
{
    istream in(data_file("test-plain-text.txt").c_str());
    std::string line;

    EXPECT_EQ(in.line(), 1);
    EXPECT_EQ(in.column(), 1);

    std::getline(in, line);
    EXPECT_EQ(line, "Hello, this is a test file.");
    EXPECT_EQ(in.line(), 2);
    EXPECT_EQ(in.column(), 1); // next line begins
}

TEST(Istream, MultipleLinesAdvanceCorrectly)
{
    istream in(data_file("test-plain-text.txt").c_str());
    std::string line;

    std::getline(in, line); // line 1
    std::getline(in, line); // line 2
    std::getline(in, line); // line 3

    EXPECT_EQ(line, "1234567890");
    EXPECT_EQ(in.line(), 4);
    EXPECT_EQ(in.column(), 1);
}

TEST(Istream, Utf8FileSupport)
{
    istream in(data_file("test-utf8.txt").c_str(), true);
    std::string line;

    std::getline(in, line);
    EXPECT_TRUE(line.find("\xE2\x82\xAC") != std::string::npos); // € in UTF-8
    EXPECT_EQ(in.line(), 2);
    EXPECT_EQ(in.column(), 1);
}

TEST(Istream, ReadToEof)
{
    istream in(data_file("test-plain-text.txt").c_str());
    std::string all;
    std::string line;

    while (std::getline(in, line))
    {
        all += line + "\n";
    }

    EXPECT_FALSE(all.empty());
    EXPECT_GE(in.line(), 4);
}

TEST(Istream, FailGracefullyIfFileMissing)
{
    istream in("/non/existent/file.txt");
    EXPECT_FALSE(in.good());
}

TEST(Istream, PeekDoesNotAdvancePosition)
{
    istream in(data_file("test-plain-text.txt").c_str());

    char c1 = static_cast<char>(in.peek());
    int line1 = in.line();
    int col1 = in.column();

    char c2 = static_cast<char>(in.get());

    EXPECT_EQ(c1, c2); // peek == get
    EXPECT_EQ(in.line(), line1);
    EXPECT_EQ(in.column(), col1 + 1);
}