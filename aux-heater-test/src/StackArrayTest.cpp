#include <gtest/gtest.h>
#include <StringStackArray.h>
#include <ByteStackArray.h>
#include <stdio.h>
#include <math.h>


TEST(StringStackArrayTest, StackArrayTestAppendUnshift)
{
	char smsSender[] = "+37211111";
	StringStackArray callHangupStack(2);

	EXPECT_EQ(callHangupStack.MaxSize(), 2);

	EXPECT_EQ(callHangupStack.Size(), 0);

    char * item = callHangupStack.AppendCopy(smsSender);

	EXPECT_EQ(callHangupStack.Size(), 1);

	item = callHangupStack.AppendCopy(smsSender);
	EXPECT_EQ(item, nullptr);

	EXPECT_EQ(callHangupStack.Size(), 1);

    item = callHangupStack.UnshiftFirst();

	EXPECT_EQ(callHangupStack.Size(), 0);
    free(item);
}

TEST(ByteStackArrayTest, ByteStackArrayTestAppendUnshift)
{
	uint8_t byteArray[5] = { 1u, 2u, 3u, 4u, 5u };
	
	ByteStackArray byteStackArray(10, 10);

	byteStackArray.Append(byteArray, 5);

	EXPECT_EQ(byteStackArray.Size(), 1);

	byteStackArray.Append(byteArray, 5);

	EXPECT_EQ(byteStackArray.Size(), 1);

	byteStackArray.Append(byteArray, 5);

	EXPECT_EQ(byteStackArray.Size(), 2);

	ByteArray * arr = byteStackArray.UnshiftFirst();

	EXPECT_EQ(arr->length, 10);

	free(arr->array);
	free(arr);

	EXPECT_EQ(byteStackArray.Size(), 1);

	arr = byteStackArray.UnshiftFirst();

	EXPECT_EQ(arr->length, 5);

	free(arr->array);
	free(arr);

	EXPECT_EQ(byteStackArray.Size(), 0);
    //free(item);
}
TEST(ByteStackArrayTest, ByteStackArrayTestSpecial)
{
	uint8_t size = 20;
	uint8_t sizePerItem = 20;
	ByteStackArray byteStackArray(size, sizePerItem);

	char msg[] = "This messsage is over 20 characters\r\n";
	uint8_t len = strlen(msg);

	byteStackArray.Append((uint8_t *)msg, len);

	EXPECT_EQ(true, byteStackArray.HasFilledPacket());

	EXPECT_EQ(byteStackArray.Size(), len / sizePerItem + 1);

	ByteArray * item = byteStackArray.UnshiftFirst();

	EXPECT_EQ(item->length, sizePerItem);

	free(item->array);
	free(item);

	item = byteStackArray.UnshiftFirst();

	EXPECT_EQ(item->length, len - sizePerItem);

	free(item->array);
	free(item);
}

TEST(ByteStackArrayTest, ByteStackArrayTestOverflow)
{
	uint8_t size = 5;
	uint8_t sizePerItem = 5;

	uint8_t arrLen = size * sizePerItem + sizePerItem;
	uint8_t * array = (uint8_t *)malloc(arrLen);

	ByteStackArray byteStackArray(size, sizePerItem);

	byteStackArray.Append(array, arrLen);

	EXPECT_EQ(true, byteStackArray.HasFilledPacket());

	// array size bigger than capacity
	EXPECT_NE(byteStackArray.Size(), arrLen / sizePerItem + 1);

	ByteArray * item = NULL;

	for (int i = 0; i < size; i++) {
		item = byteStackArray.UnshiftFirst();
		EXPECT_EQ(item->length, sizePerItem);
		free(item->array);
		free(item);
	}

	EXPECT_EQ(false, byteStackArray.HasFilledPacket());

	EXPECT_EQ(byteStackArray.Size(), 0);

}