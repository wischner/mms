#include <gtest/gtest.h>

#include <mms/mms.h>

using mms::bookmark;

TEST(Bookmark, StoresAndReturnsValues)
{
    bookmark b(42, 5, 10);

    EXPECT_EQ(b.position(), 42);
    EXPECT_EQ(b.line(), 5);
    EXPECT_EQ(b.column(), 10);
}
