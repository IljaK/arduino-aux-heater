#include <gtest/gtest.h>
#include <Util.h>


TEST(Util, SplitArgumentsTest)
{
	char source[] = "\"+37299685675\", \"Ilja aux-1\",, \"19/07/31,13:29:43+12\"";
	char *arguments[10];

	int args = SplitString(source, (char *)",", arguments, 10, false);

	EXPECT_EQ(args, 4);

	char source2[] = "\"+37299685675\", \"Ilja aux-1\",, \"19/07/31,13:29:43+12\"";
	args = SplitString(source2, (char *)",", arguments, 10, true);

	EXPECT_EQ(args, 3);
}