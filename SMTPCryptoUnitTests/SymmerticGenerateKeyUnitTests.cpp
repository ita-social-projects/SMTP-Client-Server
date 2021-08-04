// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"
#include "CppUnitTest.h"

constexpr auto EQUAL = 0;
constexpr auto IV_LEN = 16;
constexpr auto KEY_LEN = 32;
constexpr auto PASS_LEN = 8;
constexpr auto WRONG_KEY_LEN = 25;
constexpr auto WRONG_IV_LEN = 11;
constexpr auto WRONG_PASS_LEN = 6;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace SMTPCryptoUnitTests
{
	TEST_CLASS(SymmerticGenerateKeyUnitTests)
	{
	public:
		TEST_METHOD(GenerateTwoKeysFromOnePassword)
		{
			unsigned char pass[] = "password";
			int pass_len = 8;

			crpt.GenerateKeyFromPassword(pass, pass_len);
			size_t key_size = static_cast<size_t>(crpt.get_key_size());
			std::unique_ptr<unsigned char[]> key1 = std::make_unique<unsigned char[]>(key_size);
			unsigned char* key1_ptr = key1.get();

			for (size_t i = 0; i < key_size; i++) {
				*(key1_ptr + i) = *(crpt.get_aes_key() + i);
			}

			crpt.GenerateKeyFromPassword(pass, pass_len);

			unsigned char* key2_ptr = crpt.get_aes_key();

			Assert::AreNotEqual(EQUAL, strcmp((char*)key1_ptr, (char*)key2_ptr));
		}
		TEST_METHOD(GenerateKeyFromPasswordWithNullPointer)
		{
			unsigned char* pass = nullptr;
			int pass_len = PASS_LEN;
			Assert::IsFalse(crpt.GenerateKeyFromPassword(pass, pass_len));
		}
		TEST_METHOD(GenerateKeyFromPasswordWithWrongPassLength)
		{
			unsigned char pass[] = "pass";
			int pass_len = WRONG_PASS_LEN;
			Assert::IsFalse(crpt.GenerateKeyFromPassword(pass, pass_len));
		}
		TEST_METHOD(GenerateRandomKeyWithWrongKeyLength)
		{
			int key_len = WRONG_KEY_LEN;
			int iv_len = IV_LEN;
			Assert::IsFalse(crpt.GenerateRandomKey(key_len, iv_len));
		}
		TEST_METHOD(GenerateRandomKeyWithWrongIvLength)
		{
			int key_len = KEY_LEN;
			int iv_len = WRONG_IV_LEN;
			Assert::IsFalse(crpt.GenerateRandomKey(key_len, iv_len));
		}
		TEST_METHOD(GenerateRandomKeyWithCorrectLength)
		{
			int key_len = KEY_LEN;
			int iv_len = IV_LEN;
			Assert::IsTrue(crpt.GenerateRandomKey(key_len, iv_len));
		}
	private:
		SymmetricCrypto crpt;
	};
}
