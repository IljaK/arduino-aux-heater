#include <gtest/gtest.h>
#include <ByteStackArray.h>

TEST(ByteStackArrayTest, ByteStackArrayAppendTest)
{
    ByteStackArray bStackArray(8, 8);

    char data1[] = { 1, 2, 3, 4, 5, 0};
    char data2[] = { 1, 2, 3, 4, 5, 0};

    bStackArray.Append((uint8_t *)data1, sizeof(data1));
    ByteArray * dt = bStackArray.Peek();
    char * res = (char *)dt->array;

	EXPECT_EQ(bStackArray.Size(), 1);

    bStackArray.Append((uint8_t *)data2, sizeof(data2));

	EXPECT_EQ(dt->length, bStackArray.maxItemLength);
	EXPECT_EQ(bStackArray.Size(), 2);

    dt = bStackArray.Peek(1);
    res = (char *)dt->array;

    EXPECT_EQ(dt->length, 6);

    dt = bStackArray.UnshiftFirst();

    EXPECT_EQ(bStackArray.Size(), 1);

    bStackArray.FreeItem(dt);

    dt = bStackArray.UnshiftFirst();

    EXPECT_EQ(bStackArray.Size(), 0);

    bStackArray.FreeItem(dt);

}