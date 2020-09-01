#include <gtest/gtest.h>
#include <StringStackArray.h>


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