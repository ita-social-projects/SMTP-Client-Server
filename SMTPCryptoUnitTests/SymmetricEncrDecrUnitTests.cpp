// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"
#include "CppUnitTest.h"

constexpr auto EQUAL = 0;
constexpr auto RANDOM_MSG_LEN = 32;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace SMTPCryptoUnitTests
{
	TEST_CLASS(SymmetricEncrDecrUnitTests)
	{
	public:
		TEST_METHOD(EncryptAndDecryptMessageArrayTest)
		{
			unsigned char expected_msg[] = "1:Symmetric encryption and decryption of unsigned char array";
			int expected_msg_len = static_cast<int>(strlen((char*)expected_msg));

			std::shared_ptr<unsigned char[]>  encr_msg, actual_msg;

			int enc_msg_len;
			enc_msg_len = crypto.Encrypt(expected_msg, expected_msg_len, encr_msg);

			int actual_msg_len;
			actual_msg_len = crypto.Decrypt(encr_msg.get(), enc_msg_len, actual_msg);

			Assert::AreEqual(EQUAL, strcmp((char*)expected_msg, (char*)actual_msg.get()));
		}
		TEST_METHOD(EncryptMessageArrayWithNullPointerTest)
		{
			unsigned char* msg = nullptr;
			int msg_len = RANDOM_MSG_LEN;
			std::shared_ptr<unsigned char[]> encr_msg;
			Assert::AreEqual(
				(int)AsymmetricCrypto::AsymmetricErrors::E_EMPTY_MESSAGE,
				crypto.Encrypt(msg, msg_len, encr_msg));
		}
		TEST_METHOD(DecryptMessageArrayWithNullPointerTest)
		{
			unsigned char* msg = nullptr;
			int msg_len = RANDOM_MSG_LEN;
			std::shared_ptr<unsigned char[]> decr_msg;
			Assert::AreEqual(
				(int)AsymmetricCrypto::AsymmetricErrors::E_EMPTY_MESSAGE,
				crypto.Decrypt(msg, msg_len, decr_msg));
		}
		TEST_METHOD(EncryptAndDecryptMessageVectorTest)
		{
			unsigned char msg[] = "2:Symmetric encryption and decryption of unsigned char vector";
			std::vector<unsigned char> expected_vec, encr_vec, actual_vec;
			for (auto i = msg; *i != (unsigned char)'\0'; i++) {
				expected_vec.push_back(*i);
			}
			int enc_vec_len;
			enc_vec_len = crypto.Encrypt(expected_vec, encr_vec);
			encr_vec.resize(static_cast<size_t>(enc_vec_len));
			int dec_vec_len;
			dec_vec_len = crypto.Decrypt(encr_vec, actual_vec);
			actual_vec.resize(static_cast<size_t>(dec_vec_len));
			Assert::AreEqual(expected_vec.front(), actual_vec.front());
		}
		TEST_METHOD(EncryptMessageVectorWithEmptyVectorTest)
		{
			std::vector<unsigned char> plain_vec, encr_vec;
			Assert::AreEqual(
				(int)AsymmetricCrypto::AsymmetricErrors::E_EMPTY_MESSAGE,
				crypto.Encrypt(plain_vec, encr_vec));
		}
		TEST_METHOD(DecryptMessageVectorWithEmptyVectorTest)
		{
			std::vector<unsigned char> encr_vec, decr_vec;
			Assert::AreEqual(
				(int)AsymmetricCrypto::AsymmetricErrors::E_EMPTY_MESSAGE,
				crypto.Decrypt(encr_vec, decr_vec));
		}
	private:
		SymmetricCrypto crypto;
	};
}
