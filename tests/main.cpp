#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include <mms/mms.h>

namespace fs = std::filesystem;

fs::path exeDir;

// Entry point for tests to resolve path to data
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    exeDir = fs::weakly_canonical(fs::path(argv[0])).parent_path();
    return RUN_ALL_TESTS();
}