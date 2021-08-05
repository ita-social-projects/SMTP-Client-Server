// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"
#include "CppUnitTest.h"

constexpr auto EQUAL = 0;
constexpr auto RANDOM_LEN = 5;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace SMTPCryptoUnitTests
{
	TEST_CLASS(SymmetrricConstructorUnitTests)
	{
	public:

		TEST_METHOD(DefaultConstructorKeyLen)
		{
			SymmetricCrypto crpt;
			int expected_key_len = g_key_len;
			int actual_key_len = crpt.get_key_size();
			Assert::AreEqual(expected_key_len, actual_key_len);
		}
		TEST_METHOD(ConstructWithParamsNullKey)
		{
			unsigned char* key_ptr = nullptr;
			unsigned char* iv_ptr = (unsigned char*)g_iv;
			SymmetricCrypto crpt(key_ptr, g_key_len, iv_ptr, g_iv_len);
			Assert::IsNull(crpt.get_aes_key());
		}
		TEST_METHOD(ConstructWithParamsNullIv)
		{
			unsigned char* key_ptr = (unsigned char*)g_key;
			unsigned char* iv_ptr = nullptr;
			SymmetricCrypto crpt(key_ptr, g_key_len, iv_ptr, g_iv_len);
			Assert::IsNull(crpt.get_aes_key());
		}
		TEST_METHOD(ConstructWithParamsWrongKeyLen)
		{
			unsigned char* key_ptr = (unsigned char*)g_key;
			unsigned char* iv_ptr = (unsigned char*)g_iv;
			unsigned int key_len = RANDOM_LEN;
			SymmetricCrypto crpt(key_ptr, key_len, iv_ptr, g_iv_len);
			Assert::IsNull(crpt.get_aes_key());
		}
		TEST_METHOD(ConstructWithParamsWrongIvLen)
		{
			unsigned char* key_ptr = (unsigned char*)g_key;
			unsigned char* iv_ptr = (unsigned char*)g_iv;
			unsigned int iv_len = RANDOM_LEN;
			SymmetricCrypto crpt(key_ptr, g_key_len, iv_ptr, iv_len);
			Assert::IsNull(crpt.get_aes_key());
		}
	};
}
