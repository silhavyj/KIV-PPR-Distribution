#include <gtest/gtest.h>

#include "myproject/math.h"

TEST(add_1, Positive)
{
    EXPECT_EQ(1, MyMath::add(0, 1));
}

TEST(add_2, Positive)
{
    EXPECT_EQ(-4, MyMath::add(-5, 1));
}