#include <gtest/gtest.h>
#include <filesystem>
#include <string>
#include <mms/mms.h>

using mms::streambuf;
namespace fs = std::filesystem;

extern fs::path exeDir;

class testbuf : public mms::streambuf
{
public:
    using mms::streambuf::sputbackc;
    using mms::streambuf::tracker;
    using mms::streambuf::uflow;
    testbuf(const char *filename) : streambuf(filename) {}
};

static fs::path data_file(const std::string &name)
{
    return exeDir / "data" / name;
}

TEST(Streambuf, BasicReading)
{
    testbuf buf(data_file("test-plain-text.txt").c_str());
    EXPECT_EQ(buf.tracker().line(), 1);
    EXPECT_EQ(buf.tracker().column(), 1);

    char c;
    for (int i = 0; i < 5; ++i)
    {
        c = buf.uflow();
    }
    EXPECT_EQ(buf.tracker().line(), 1);
    EXPECT_EQ(buf.tracker().column(), 6);
}

TEST(Streambuf, LineTracking)
{
    testbuf buf(data_file("test-plain-text.txt").c_str());
    char c;
    while ((c = buf.uflow()) != EOF)
    {
        if (c == '\n')
            break;
    }

    EXPECT_EQ(buf.tracker().line(), 2);
    EXPECT_EQ(buf.tracker().column(), 1);
}

TEST(Streambuf, BlockRead)
{
    testbuf buf(data_file("test-plain-text.txt").c_str());
    char buffer[64] = {};
    std::streamsize n = buf.sgetn(buffer, 20);
    EXPECT_EQ(n, 20);

    std::string read_data(buffer, n);
    EXPECT_TRUE(read_data.starts_with("Hello"));
    EXPECT_EQ(buf.tracker().line(), 1);
    EXPECT_EQ(buf.tracker().column(), 21);
}

TEST(Streambuf, UTF8ContentHandling)
{
    testbuf buf(data_file("test-utf8.txt").c_str());

    std::string first_line;
    char c;
    while ((c = buf.uflow()) != '\n')
        first_line.push_back(c);

    EXPECT_EQ(buf.tracker().line(), 2);
    EXPECT_EQ(buf.tracker().column(), 1);
    EXPECT_TRUE(first_line.find("\xE2\x82\xAC") != std::string::npos); // € in UTF-8
}

TEST(Streambuf, EOFBehavior)
{
    testbuf buf(data_file("test-plain-text.txt").c_str());
    int count = 0;
    while (buf.uflow() != EOF)
        ++count;

    EXPECT_GT(count, 0);
    EXPECT_EQ(buf.uflow(), EOF);
}
