#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include <mms/mms.h>

namespace fs = std::filesystem;
using mms::file;

extern fs::path exeDir;

// Helper: construct path to test file in bin/data/
static fs::path data_file(const std::string &name)
{
    return exeDir / "data" / name;
}

// Helper: read file as string
static std::string read_file(const fs::path &path)
{
    std::ifstream in(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(in), {});
}

TEST(MappedFile, PlainTextIsCorrectlyMapped)
{
    auto path = data_file("test-plain-text.txt");
    ASSERT_TRUE(fs::exists(path));

    std::string expected = read_file(path);

    file f(path.c_str());

    ASSERT_TRUE(f.is_open());
    ASSERT_EQ(f.size(), expected.size());

    std::string mapped(f.data(), f.size());
    EXPECT_EQ(mapped, expected);
}

TEST(MappedFile, UTF8FileIsCorrectlyMapped)
{
    auto path = data_file("test-utf8.txt");
    ASSERT_TRUE(fs::exists(path));

    std::string expected = read_file(path);

    file f(path.c_str());

    ASSERT_TRUE(f.is_open());
    ASSERT_EQ(f.size(), expected.size());

    std::string mapped(f.data(), f.size());
    EXPECT_EQ(mapped, expected);

    // Spot-check first few characters (UTF-8 aware)
    EXPECT_TRUE(mapped.starts_with("€uro"));
    EXPECT_NE(mapped.find("𐍈"), std::string::npos); // Gothic letter
}

TEST(MappedFile, InvalidPathThrowsException)
{
    EXPECT_THROW(file("data/this-file-does-not-exist.txt"), std::ios_base::failure);
}
