#include <gtest/gtest.h>
#include "common/Util.h"

TEST(UtilTest, SplitArgumentsTest)
{
	char source[] = "\"+37299685675\", \"Ilja aux-1\",, \"19/07/31,13:29:43+12\"";
	char *arguments[10];

	int args = SplitString(source, (char *)",", arguments, 10, false);

	EXPECT_EQ(args, 4);

	char source2[] = "\"+37299685675\", \"Ilja aux-1\",, \"19/07/31,13:29:43+12\"";
	args = SplitString(source2, (char *)",", arguments, 10, true);

	EXPECT_EQ(args, 3);
}

TEST(UtilTest, Bit8ValueTest)
{
	uint8_t expect = 235; // 11101011

	uint8_t input = 0xff;
	uint8_t inVal = 2; // 10 binary

	setBitsValue(&input, inVal, 3, 2);

	EXPECT_EQ(input, expect);

	uint8_t outVal = getBitsValue(&input, 3, 2);

	EXPECT_EQ(inVal, outVal);

	EXPECT_EQ(input, expect);
}

TEST(UtilTest, Bit16ValueTest)
{
	uint16_t expect = 235; // 11101011

	uint16_t input = 0xff;
	uint16_t inVal = 2; // 10 binary

	setBitsValue(&input, inVal, 3, 2);

	EXPECT_EQ(input, expect);

	uint16_t outVal = getBitsValue(&input, 3, 2);

	EXPECT_EQ(inVal, outVal);

	EXPECT_EQ(input, expect);
}

TEST(UtilTest, ReverseByteTest)
{
    EXPECT_EQ(reverseByte(0b00001111), 0b11110000);

    EXPECT_EQ(reverseByte(0b01010101), 0b10101010);
}


TEST(UtilTest, HexEncodingTest)
{
    char data[] = "some test data";
    char hex[128];
    size_t encoded = encodeToHex((uint8_t *)data, strlen(data), hex);

    EXPECT_EQ(encoded / 2, strlen(data));

    char result[128];
    size_t decoded = decodeFromHex(hex, (uint8_t *)result, encoded / 2);

    EXPECT_EQ(strcmp(data, result), 0);

}