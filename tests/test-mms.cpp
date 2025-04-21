#include <gtest/gtest.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <fstream>
#include <iterator>

#include <mms/mms.h>

namespace fs = std::filesystem;

static fs::path exeDir;

// Helper: construct full path to a data file
static fs::path data_file(const std::string &filename)
{
    return exeDir / "data" / filename;
}

// Helper: read entire file into a string
static std::string slurp(const std::string &path)
{
    std::ifstream in(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), {}};
}

TEST(PlainText, BasicLineColumnTracking)
{
    auto filepath = data_file("test-plain-text.txt");
    std::cerr << "[DEBUG] exeDir = " << exeDir << std::endl;
    std::cerr << "[DEBUG] filepath = " << filepath << std::endl;
    std::cerr << "[DEBUG] exists? " << fs::exists(filepath) << std::endl;

    mms::istream in(filepath.c_str(), /*utf8=*/false);
    std::cerr << "[DEBUG] opened stream" << std::endl;

    EXPECT_EQ(in.line(), 1);
    EXPECT_EQ(in.column(), 0);

    std::string line;
    std::getline(in, line);
    EXPECT_EQ(line, "Hello, this is a test file.");

    EXPECT_EQ(in.line(), 1);
    EXPECT_EQ(in.column(), static_cast<int>(line.size()));
}

TEST(PlainText, BookmarkAndSeek)
{
    auto filepath = data_file("test-plain-text.txt");
    std::cerr << "[DEBUG] Booking test filepath = " << filepath << std::endl;
    std::cerr << "[DEBUG] exists? " << fs::exists(filepath) << std::endl;

    mms::istream in(filepath.c_str());
    std::cerr << "[DEBUG] opened stream for bookmark test" << std::endl;

    std::string l1, l2;
    std::getline(in, l1);
    std::getline(in, l2);

    auto pos = in.tellg();
    in.rdbuf()->sputbackc('X');
    in.rdbuf()->sputbackc('1');

    in.clear();
    in.seekg(pos);
    EXPECT_EQ(in.line(), 3);
    EXPECT_EQ(in.column(), 0);
}

TEST(Utf8, UTF8DecodingAdvances)
{
    auto filepath = data_file("test-utf8.txt");
    std::cerr << "[DEBUG] UTF8 test filepath = " << filepath << std::endl;
    std::cerr << "[DEBUG] exists? " << fs::exists(filepath) << std::endl;

    mms::istream in(filepath.c_str(), /*utf8=*/true);
    std::cerr << "[DEBUG] opened UTF8 stream" << std::endl;

    auto cp = static_cast<char32_t>(in.rdbuf()->sgetc());
    EXPECT_EQ(cp, U'€');
    in.get();
    EXPECT_EQ(in.line(), 1);
    EXPECT_EQ(in.column(), 1);
}

int main(int argc, char **argv)
{
    exeDir = fs::weakly_canonical(fs::path(argv[0])).parent_path();
    std::cerr << "[DEBUG] exeDir in main = " << exeDir << std::endl;
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
