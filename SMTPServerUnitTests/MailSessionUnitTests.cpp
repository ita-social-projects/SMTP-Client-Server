// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"
#include "CppUnitTest.h"

#include "../SMTPServer/MailSession.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace MailSessionUnitTests
{
	TEST_CLASS(MailSessionUnitTests)
	{
	public:

		TEST_METHOD(TestMethodGetClientSocket)
		{
			MailSession obj(0);
			auto result = obj.get_client_socket();

			Assert::AreEqual((int)result, 0);
		}

		TEST_METHOD(TestMethodSendResponse)
		{
			MailSession obj(0);
			int result = obj.SendResponse(220);

			Assert::AreEqual(result, 220);
		}

		TEST_METHOD(TestMethodProccessHeloOk)
		{
			MailSession obj(0);
			int result = obj.Processes("HELO google.com");

			Assert::AreEqual(result, (int)Responses::OK);
		}

		TEST_METHOD(TestMethodProccessHeloSyntaxError)
		{
			MailSession obj(0);
			int result = obj.Processes("HELO googlecom");

			Assert::AreEqual(result, (int)Responses::SYNTAX_ERROR);
		}

		TEST_METHOD(TestMethodProccessEhloOk)
		{
			MailSession obj(0);
			int result = obj.Processes("EHLO google.com");

			Assert::AreEqual(result, (int)Responses::OK);
		}

		TEST_METHOD(TestMethodProccessEhloSyntaxError)
		{
			MailSession obj(0);
			int result = obj.Processes("EHLO google.com");

			Assert::AreEqual(result, (int)Responses::OK);
		}

		TEST_METHOD(TestMethodProccessAuthLogin)
		{
			MailSession obj(0);
			int result_helo = obj.Processes("EHLO google.com");
			int result_auth = obj.Processes("AUTH LOGIN");

			Assert::AreEqual(result_auth, (int)Responses::LOGIN_RCV);
		}

		TEST_METHOD(TestMethodProccessAuthLoginRecieve)
		{
			MailSession obj(0);
			int result_helo = obj.Processes("EHLO google.com");
			int result_auth = obj.Processes("AUTH LOGIN");
			int result_auth_login = obj.Processes("login@gmail.com");

			Assert::AreEqual(result_auth_login, (int)Responses::LOGIN_RCV);
		}

		//TEST_METHOD(TestMethodProccessAuthPasswordRecieve)
		//{
		//	MailSession obj(0);
		//	int result_helo = obj.Processes("EHLO google.com");
		//	int result_auth = obj.Processes("AUTH LOGIN");
		//	int result_auth_login = obj.Processes("login@gmail.com");
		//	int result_auth_pass = obj.Processes("password");

		//	Assert::AreEqual(result_auth_pass, (int)Responses::LOGIN_SUCCESS);
		//}

		TEST_METHOD(TestMethodProccessMailBadSequence)
		{
			MailSession obj(0);
			int result_helo = obj.Processes("EHLO google.com");
			int result_mail = obj.Processes("MAIL <login@gmail.com>");

			Assert::AreEqual(result_mail, (int)Responses::BAD_SEQUENSE);
		}

		//TEST_METHOD(TestMethodProccessMailOk)
		//{
		//	MailSession obj(0);
		//	int result_helo = obj.Processes("EHLO google.com");
		//	int result_auth = obj.Processes("AUTH LOGIN");
		//	int result_auth_login = obj.Processes("login@gmail.com");
		//	int result_auth_pass = obj.Processes("password");
		//	int result_mail = obj.Processes("MAIL <login@gmail.com>");

		//	Assert::AreEqual(result_mail, (int)Responses::OK);
		//}

		//TEST_METHOD(TestMethodProccessMailSyntaxError)
		//{
		//	MailSession obj(0);
		//	int result_helo = obj.Processes("EHLO google.com");
		//	int result_auth = obj.Processes("AUTH LOGIN");
		//	int result_auth_login = obj.Processes("login@gmail.com");
		//	int result_auth_pass = obj.Processes("password");
		//	int result_mail = obj.Processes("MAIL <logingmail.com>");

		//	Assert::AreEqual(result_mail, (int)Responses::SYNTAX_ERROR);
		//}

		TEST_METHOD(TestMethodProccessRcptBadSequence)
		{
			MailSession obj(0);
			int result_helo = obj.Processes("EHLO google.com");
			int result_rcpt = obj.Processes("RCPT <login@gmail.com>");

			Assert::AreEqual(result_rcpt, (int)Responses::BAD_SEQUENSE);
		}

		//TEST_METHOD(TestMethodProccessRcptOk)
		//{
		//	MailSession obj(0);
		//	int result_helo = obj.Processes("EHLO google.com");
		//	int result_auth = obj.Processes("AUTH LOGIN");
		//	int result_auth_login = obj.Processes("login@gmail.com");
		//	int result_auth_pass = obj.Processes("password");
		//	int result_mail = obj.Processes("MAIL <login@gmail.com>");
		//	int result_rcpt = obj.Processes("RCPT <login@gmail.com>");

		//	Assert::AreEqual(result_rcpt, (int)Responses::OK);
		//}

		//TEST_METHOD(TestMethodProccessRcptSyntaxError)
		//{
		//	MailSession obj(0);
		//	int result_helo = obj.Processes("EHLO google.com");
		//	int result_auth = obj.Processes("AUTH LOGIN");
		//	int result_auth_login = obj.Processes("login@gmail.com");
		//	int result_auth_pass = obj.Processes("password");
		//	int result_mail = obj.Processes("MAIL <login@gmail.com>");
		//	int result_rcpt = obj.Processes("RCPT <logingmail.com>");

		//	Assert::AreEqual(result_rcpt, (int)Responses::SYNTAX_ERROR);
		//}

		TEST_METHOD(TestMethodProccessDataBadSequence)
		{
			MailSession obj(0);
			int result_helo = obj.Processes("EHLO google.com");
			int result_data = obj.Processes("DATA");

			Assert::AreEqual(result_data, (int)Responses::BAD_SEQUENSE);
		}

		//TEST_METHOD(TestMethodProccessDataOk)
		//{
		//	MailSession obj(0);
		//	int result_helo = obj.Processes("EHLO google.com");
		//	int result_auth = obj.Processes("AUTH LOGIN");
		//	int result_auth_login = obj.Processes("login@gmail.com");
		//	int result_auth_pass = obj.Processes("password");
		//	int result_mail = obj.Processes("MAIL <login@gmail.com>");
		//	int result_rcpt = obj.Processes("RCPT <login@gmail.com>");
		//	int result_data = obj.Processes("DATA");

		//	Assert::AreEqual(result_data, (int)Responses::START_MAIL);
		//}

		//TEST_METHOD(TestMethodSubProccessSubjectOk)
		//{
		//	MailSession obj(0);
		//	int result_helo = obj.Processes("EHLO google.com");
		//	int result_auth = obj.Processes("AUTH LOGIN");
		//	int result_auth_login = obj.Processes("login@gmail.com");
		//	int result_auth_pass = obj.Processes("password");
		//	int result_mail = obj.Processes("MAIL <login@gmail.com>");
		//	int result_rcpt = obj.Processes("RCPT <login@gmail.com>");
		//	int result_data = obj.Processes("DATA");
		//	int result_subject = obj.Processes("Subject: Hello!");

		//	Assert::AreEqual(result_subject, (int)Responses::OK);
		//}

		//TEST_METHOD(TestMethodSubProccessSubjectSyntaxError)
		//{
		//	MailSession obj(0);
		//	int result_helo = obj.Processes("EHLO google.com");
		//	int result_auth = obj.Processes("AUTH LOGIN");
		//	int result_auth_login = obj.Processes("login@gmail.com");
		//	int result_auth_pass = obj.Processes("password");
		//	int result_mail = obj.Processes("MAIL <login@gmail.com>");
		//	int result_rcpt = obj.Processes("RCPT <login@gmail.com>");
		//	int result_data = obj.Processes("DATA");
		//	int result_subject = obj.Processes("Hello!");

		//	Assert::AreEqual(result_subject, (int)Responses::SYNTAX_ERROR);
		//}

		//TEST_METHOD(TestMethodSubProccessEmailOk)
		//{
		//	MailSession obj(0);
		//	int result_helo = obj.Processes("EHLO google.com");
		//	int result_auth = obj.Processes("AUTH LOGIN");
		//	int result_auth_login = obj.Processes("login@gmail.com");
		//	int result_auth_pass = obj.Processes("password");
		//	int result_mail = obj.Processes("MAIL <login@gmail.com>");
		//	int result_rcpt = obj.Processes("RCPT <login@gmail.com>");
		//	int result_data = obj.Processes("DATA");
		//	int result_subject = obj.Processes("Subject: Hello!");
		//	int result_email = obj.Processes((char*)SMTP_DATA_TERMINATOR);

		//	Assert::AreEqual(result_email, 1);
		//}

		//TEST_METHOD(TestMethodSubProccessEmailNotOk)
		//{
		//	MailSession obj(0);
		//	int result_helo = obj.Processes("EHLO google.com");
		//	int result_auth = obj.Processes("AUTH LOGIN");
		//	int result_auth_login = obj.Processes("login@gmail.com");
		//	int result_auth_pass = obj.Processes("password");
		//	int result_mail = obj.Processes("MAIL <login@gmail.com>");
		//	int result_rcpt = obj.Processes("RCPT <login@gmail.com>");
		//	int result_data = obj.Processes("DATA");
		//	int result_subject = obj.Processes("Subject: Hello!");
		//	int result_email = obj.Processes("Hello!");

		//	Assert::AreEqual(result_email, (int)Responses::EMAIL_N_RECEIVED);
		//}

		//TEST_METHOD(TestMethodProccessQUIT)
		//{
		//	MailSession obj(0);
		//	int result_helo = obj.Processes("EHLO google.com");
		//	int result_auth = obj.Processes("AUTH LOGIN");
		//	int result_auth_login = obj.Processes("login@gmail.com");
		//	int result_auth_pass = obj.Processes("password");
		//	int result_mail = obj.Processes("MAIL <login@gmail.com>");
		//	int result_rcpt = obj.Processes("RCPT <login@gmail.com>");
		//	int result_data = obj.Processes("DATA");
		//	int result_subject = obj.Processes("Subject: Hello!");
		//	int result_email = obj.Processes((char*)SMTP_DATA_TERMINATOR);
		//	int result_quit = obj.Processes("QUIT");

		//	Assert::AreEqual(result_quit, (int)Responses::SERVICE_CLOSING);
		//}
	};
}

