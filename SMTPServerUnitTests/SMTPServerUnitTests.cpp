// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"
#include "CppUnitTest.h"

#include "../SMTPServer/SMTPServer.cpp"
#include "../SMTPServer/MailSession.cpp"
#include "../SMTPServer/ThreadPool.cpp"
#include "../SMTPServer/MailInfo.cpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace SMTPServerUnitTests
{
	TEST_CLASS(SMTPServerUnitTests)
	{
	public:

		TEST_METHOD(TestMethodInitialize)
		{
			SMTPServer obj;
			auto result_init = obj.Initialize();

			Assert::IsTrue(result_init);
		}

		TEST_METHOD(TestMethodSetSocketSettings)
		{
			SMTPServer obj;
			auto result_init = obj.Initialize();
			auto result_settings = obj.SetSocketSettings();

			Assert::IsTrue(result_settings);
		}

		TEST_METHOD(TestMethodServerStart)
		{
			SMTPServer obj;
			auto result_init = obj.Initialize();
			auto result_settings = obj.SetSocketSettings();
			auto result_starting = obj.ServerStart();

			Assert::IsTrue(result_starting);
		}
	};
}

