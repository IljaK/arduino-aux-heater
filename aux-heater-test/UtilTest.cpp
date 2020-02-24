#include "stdafx.h"
#include "CppUnitTest.h"
#include "../aux-heater/Util.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace AudiControlUnitTest
{
	TEST_CLASS(UtilTest)
	{
	public:

		TEST_METHOD(SplitArgumentsTest)
		{
			char source[] = "\"+37299685675\", \"Ilja aux-1\",, \"19/07/31,13:29:43+12\"";
			char *arguments[10];

			int args = SplitString(source, ",", arguments, 10, false);
			wchar_t message[128];

			if (args != 4) {
				swprintf(message, 128, L"Wrong splitted arguments amount! %d", args);
				Assert::Fail(message);
			}

			char source2[] = "\"+37299685675\", \"Ilja aux-1\",, \"19/07/31,13:29:43+12\"";
			args = SplitString(source2, ",", arguments, 10, true);

			if (args != 3) {
				swprintf(message, 128, L"Wrong splitted arguments amount without empty! %d", args);
				Assert::Fail(message);
			}
		}
	};
}