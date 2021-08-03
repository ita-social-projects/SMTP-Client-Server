#include "pch.h"
#include "CppUnitTest.h"
#include "SMTPClient.cpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace SMTPClientUnitTests
{
	TEST_CLASS(SMTPClientUnitTests)
	{		
	public:
		
		TEST_METHOD(SetLoginTrue)
		{	
			SMTPClientClass obj;
			Assert::IsTrue(obj.set_login("someemail@gmail.com"));			
		}
		TEST_METHOD(SetLoginThrow)
		{
			SMTPClientClass obj;
			auto func = [&] {obj.set_login(""); };
			Assert::ExpectException<SMTPErrorClass>(func);			
		}

		TEST_METHOD(SetLoginThrowValue)
		{
			SMTPClientClass obj;
			try
			{
				obj.set_login("");
			}
			catch (SMTPErrorClass& er)
			{
				Assert::AreEqual(std::string("Couldn't set value, because argument was empty string"), er.GetErrorText());
			}
		}

		TEST_METHOD(SetPortTrue)
		{
			SMTPClientClass obj;
			Assert::IsTrue(obj.set_port("465"));
		}

		TEST_METHOD(SetPortThrow)
		{
			SMTPClientClass obj;
			auto func = [&] {obj.set_port(""); };
			Assert::ExpectException<SMTPErrorClass>(func);
		}

		TEST_METHOD(SetPortThrowValue)
		{
			SMTPClientClass obj;
			try
			{
				obj.set_port("");
			}
			catch (SMTPErrorClass& er)
			{
				Assert::AreEqual(std::string("Couldn't set value, because argument was empty string"), er.GetErrorText());
			}
		}

		TEST_METHOD(SetPasswordTrue)
		{
			SMTPClientClass obj;
			Assert::IsTrue(obj.set_password("somepassword"));
		}
		TEST_METHOD(SetPasswordThrow)
		{
			SMTPClientClass obj;
			auto func = [&] {obj.set_password(""); };
			Assert::ExpectException<SMTPErrorClass>(func);
		}

		TEST_METHOD(SetPasswordThrowValue)
		{
			SMTPClientClass obj;
			try
			{
				obj.set_password("");
			}
			catch (SMTPErrorClass& er)
			{
				Assert::AreEqual(std::string("Couldn't set value, because argument was empty string"), er.GetErrorText());
			}
		}

		TEST_METHOD(SetRecepientEmailTrue)
		{
			SMTPClientClass obj;
			Assert::IsTrue(obj.set_recepient_email("someemail@gmail.com"));
		}
		TEST_METHOD(SetRecepientEmailThrow)
		{
			SMTPClientClass obj;
			auto func = [&] {obj.set_recepient_email(""); };
			Assert::ExpectException<SMTPErrorClass>(func);
		}

		TEST_METHOD(SetRecepientEmailValue)
		{
			SMTPClientClass obj;
			try
			{
				obj.set_recepient_email("");
			}
			catch (SMTPErrorClass& er)
			{
				Assert::AreEqual(std::string("Couldn't set value, because argument was empty string"), er.GetErrorText());
			}
		}

		TEST_METHOD(SetLetterMessageTrue)
		{
			SMTPClientClass obj;
			Assert::IsTrue(obj.set_letter_message("some letter"));
		}
		TEST_METHOD(SetLetterMessageThrow)
		{
			SMTPClientClass obj;
			auto func = [&] {obj.set_letter_message(""); };
			Assert::ExpectException<SMTPErrorClass>(func);
		}

		TEST_METHOD(SetLetterMessageThrowValue)
		{
			SMTPClientClass obj;
			try
			{
				obj.set_letter_message("");
			}
			catch (SMTPErrorClass& er)
			{
				Assert::AreEqual(std::string("Couldn't set value, because argument was empty string"), er.GetErrorText());
			}
		}

		TEST_METHOD(SetSMTPAddressTrue)
		{
			SMTPClientClass obj;
			Assert::IsTrue(obj.set_smtp_address("some_smtp_server_address"));
		}
		TEST_METHOD(SetSMTPAddressThrow)
		{
			SMTPClientClass obj;
			auto func = [&] {obj.set_smtp_address(""); };
			Assert::ExpectException<SMTPErrorClass>(func);
		}

		TEST_METHOD(SetSMTPAddressThrowValue)
		{
			SMTPClientClass obj;
			try
			{
				obj.set_smtp_address("");
			}
			catch (SMTPErrorClass& er)
			{
				Assert::AreEqual(std::string("Couldn't set value, because argument was empty string"), er.GetErrorText());
			}
		}
	};	
}
