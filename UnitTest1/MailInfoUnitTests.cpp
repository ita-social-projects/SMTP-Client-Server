// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"
#include "CppUnitTest.h"

#include "../SMTPServer/MailInfo.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace MailInfoUnitTests
{
	TEST_CLASS(MailInfoUnitTests)
	{
	public:

		TEST_METHOD(TestMethodSetGetLogin)
		{
			std::string login = "login";
			obj.set_login(login);

			Assert::AreEqual(login, obj.get_login());
		}

		TEST_METHOD(TestMethodSetGetPassword)
		{
			std::string password = "password";
			obj.set_password(password);

			Assert::AreEqual(password, obj.get_password());
		}

		TEST_METHOD(TestMethodSetGetMailFrom)
		{
			std::string mail_from = "mail_from@gmail.com";
			obj.set_mail_from(mail_from);

			Assert::AreEqual(mail_from, obj.get_mail_from());
		}

		TEST_METHOD(TestMethodSetGetRcptTo)
		{
			std::string rcpt_to = "rcpt_to@gmail.com";
			obj.set_rcpt_to(rcpt_to);

			Assert::AreEqual(rcpt_to, obj.get_rcpt_to());
		}

		TEST_METHOD(TestMethodSetGetSubject)
		{
			std::string subject = "subject";
			obj.set_subject(subject);

			Assert::AreEqual(subject, obj.get_subject());
		}

		TEST_METHOD(TestMethodSetGetText)
		{
			std::string text = "Some text here....";
			obj.set_text(text);

			Assert::AreEqual("\n" + text, obj.get_text());
		}

	private:
		MailInfo obj;
	};

}