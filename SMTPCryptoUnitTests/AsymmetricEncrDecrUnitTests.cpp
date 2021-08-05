// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"
#include "CppUnitTest.h"

constexpr auto EQUAL = 0;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace SMTPCryptoUnitTests
{
	TEST_CLASS(AsymmetricEncrDecrUnitTests)
	{
	public:
		TEST_METHOD_INITIALIZE(Init)
		{
			crypto.GenerateRsaKeypair();

		}
		TEST_METHOD(AsymEncryptAndDecryptMessageArrayTest)
		{
			unsigned char expected_msg[] = "3:Asymmetric encryption and decryption of unsigned char array";
			int expected_msg_len = strlen((char*)expected_msg);

			std::shared_ptr<unsigned char[]> encr_msg, actual_msg;

			unsigned int encr_msg_len;
			encr_msg_len = crypto.Encrypt(expected_msg, expected_msg_len, encr_msg, key_array, &key_len, iv_array, &iv_len);

			unsigned int decr_msg_len;
			decr_msg_len = crypto.Decrypt(encr_msg.get(), encr_msg_len, key_array.get(), key_len, iv_array.get(), iv_len, actual_msg);

			Assert::AreEqual(EQUAL, strcmp((char*)expected_msg, (char*)actual_msg.get()));
		}
		TEST_METHOD(AsymEncryptMessageArrayWithNullPointerTest)
		{
			unsigned char* expected_msg = nullptr;
			int expected_msg_len = 33;

			std::shared_ptr<unsigned char[]> encr_msg;

			Assert::AreEqual(
				(int)AsymmetricCrypto::AsymmetricErrors::E_EMPTY_MESSAGE,
				crypto.Encrypt(expected_msg, expected_msg_len, encr_msg, key_array, &key_len, iv_array, &iv_len));
		}
		TEST_METHOD(AsymDecryptMessageArrayWithNullPointerTest)
		{
			unsigned char* msg = nullptr;
			int msg_len = 33;

			std::shared_ptr<unsigned char[]> decr_msg;

			Assert::AreEqual(
				(int)AsymmetricCrypto::AsymmetricErrors::E_EMPTY_MESSAGE,
				crypto.Decrypt(msg, msg_len, key_array.get(), key_len, iv_array.get(), iv_len, decr_msg));
		}
		TEST_METHOD(AsymEncryptAndDecryptMessageVectorTest)
		{

			unsigned char msg[] = "4:Asymmetric encryption and decryption of unsigned char vector";
			std::vector<unsigned char> expected_vec, encr_vec, actual_vec;

			for (auto i = msg; *i != (unsigned char)'\0'; i++) {
				expected_vec.push_back(*i);
			}

			int encr_vec_len;
			encr_vec_len = crypto.Encrypt(expected_vec, (int)expected_vec.size(), encr_vec, key_vector, &key_len, iv_vector, &iv_len);

			encr_vec.resize(static_cast<size_t>(encr_vec_len));
			int decr_vec_len;

			decr_vec_len = crypto.Decrypt(encr_vec, encr_vec_len, actual_vec, key_vector, key_len, iv_vector, iv_len);

			Assert::AreEqual(expected_vec.front(), actual_vec.front());
		}
		TEST_METHOD(AsymAsymEncryptMessageVectorWithEmptyVectorTest)
		{
			std::vector<unsigned char> plain_vec, encr_vec;

			Assert::AreEqual(
				(int)AsymmetricCrypto::AsymmetricErrors::E_EMPTY_MESSAGE,
				crypto.Encrypt(plain_vec, plain_vec.size(), encr_vec, key_vector, &key_len, iv_vector, &iv_len));
		}
		TEST_METHOD(DecryptMessageVectorWithEmptyVectorTest)
		{
			int encr_vec_len = 33;
			std::vector<unsigned char> encr_vec, decr_vec;

			Assert::AreEqual(
				(int)AsymmetricCrypto::AsymmetricErrors::E_EMPTY_MESSAGE,
				crypto.Decrypt(encr_vec, encr_vec_len, decr_vec, key_vector, key_len, iv_vector, iv_len));
		}
	private:
		AsymmetricCrypto crypto;
		unsigned int key_len = 0;
		unsigned int iv_len = 0;
		std::shared_ptr<unsigned char[]> key_array;
		std::shared_ptr<unsigned char[]> iv_array;
		std::vector<unsigned char> key_vector;
		std::vector<unsigned char> iv_vector;
	};
}
