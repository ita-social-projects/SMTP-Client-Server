#pragma once

#ifndef ICRYPTO_H
#define ICRYPTO_H

#include <vector>
#include <memory>

class ICrypt
{
public:
	virtual int EncryptSymmetric(
		const unsigned char* msg,
		unsigned int msg_len,
		std::shared_ptr<unsigned char>& encr_msg) = 0;

	virtual int EncryptSymmetric(
		const std::vector<unsigned char>& msg,
		std::vector<unsigned char>& encr_msg) = 0;

	virtual int DecryptSymmetric(
		const unsigned char* encr_msg,
		unsigned int encr_msg_len,
		std::shared_ptr<unsigned char>& decr_msg) = 0;

	virtual int DecryptSymmetric(
		const std::vector<unsigned char>& encr_msg,
		std::vector<unsigned char>& decr_msg) = 0;

	virtual bool GenerateRandomKey(
		unsigned int key_len,
		unsigned int iv_len) = 0;

	virtual bool GenerateKeyFromPassword(
		const unsigned char* password,
		unsigned int password_len) = 0;
};

#endif // ICRYPTO_H