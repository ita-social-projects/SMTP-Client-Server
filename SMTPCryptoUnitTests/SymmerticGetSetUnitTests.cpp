// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"
#include "CppUnitTest.h"

constexpr auto EQUAL = 0;
constexpr auto IV_LEN = 16;
constexpr auto KEY_LEN_16 = 16;
constexpr auto KEY_LEN_24 = 24;
constexpr auto KEY_LEN_32 = 32;
constexpr auto WRONG_LEN = 7;


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace SMTPCryptoUnitTests
{
	TEST_CLASS(SymmerticGetSetUnitTests)
	{
	public:
		TEST_METHOD(GetDefaultKeyTest)
		{
			unsigned char* expected_key = (unsigned char*)g_key;
			unsigned char* actual_key = crpt.get_aes_key();
			Assert::AreEqual(EQUAL, strcmp((char*)expected_key, (char*)actual_key));
		}
		TEST_METHOD(GetDefaultIvTest)
		{
			unsigned char* expected_iv = (unsigned char*)g_iv;
			unsigned char* actual_iv = crpt.get_aes_iv();
			Assert::AreEqual(EQUAL, strcmp((char*)expected_iv, (char*)actual_iv));
		}
		TEST_METHOD(GetDefaultKeyLenTest)
		{
			int expected_key_len = g_key_len;
			int actual_key_len = crpt.get_key_size();
			Assert::AreEqual(expected_key_len, actual_key_len);
		}
		TEST_METHOD(GetDefaultIvLenTest)
		{
			int expected_iv_len = g_iv_len;
			int actual_iv_len = crpt.get_iv_size();
			Assert::AreEqual(expected_iv_len, actual_iv_len);
		}
		TEST_METHOD(SetKeyTest)
		{
			unsigned char expected_key[] = "eirvup434o930eci9ne4u0tsdrvewq23";
			unsigned int key_len = KEY_LEN_32;
			crpt.set_aes_key(expected_key, key_len);
			unsigned char* actual_key = crpt.get_aes_key();
			Assert::AreEqual(EQUAL, strcmp((char*)expected_key, (char*)actual_key));
		}
		TEST_METHOD(SetIvTest)
		{
			unsigned char expected_iv[] = "eie4u0tsdrvewq23";
			unsigned int iv_len = IV_LEN;
			crpt.set_aes_iv(expected_iv, iv_len);
			unsigned char* actual_iv = crpt.get_aes_iv();
			Assert::AreEqual(EQUAL, strcmp((char*)expected_iv, (char*)actual_iv));
		}
		TEST_METHOD(SetWrongIvLenTest)
		{
			Assert::IsFalse(crpt.set_aes_iv((unsigned char*)"serger", WRONG_LEN));
		}
		TEST_METHOD(SetCorrectIvLenTest)
		{
			Assert::IsTrue(crpt.set_aes_iv((unsigned char*)"etryuyuiytrewerr", IV_LEN));
		}
		TEST_METHOD(SetWrongKeyLenTest)
		{
			Assert::IsFalse(crpt.set_aes_key((unsigned char*)"serger", WRONG_LEN));
		}
		TEST_METHOD(SetFirstCorrectKeyLenTest)
		{
			Assert::IsTrue(crpt.set_aes_key((unsigned char*)"etryuyuiytrewerr", KEY_LEN_16));
		}
		TEST_METHOD(SetSecondCorrectKeyLenTest)
		{
			Assert::IsTrue(crpt.set_aes_key((unsigned char*)"etryuyuiytrewerrewferfer", KEY_LEN_24));
		}
		TEST_METHOD(SetThirdCorrectKeyLenTest)
		{
			Assert::IsTrue(crpt.set_aes_key((unsigned char*)"etryuyuiytrewerretryuyuiytrewerr", KEY_LEN_32));
		}
	private:
		SymmetricCrypto crpt;
	};
}
