#pragma once

#include <vector>
#include <memory>

#include <openssl/pem.h>    // engine interface
#include <openssl/aes.h>    // AES_BLOCK_SIZE
#include <openssl/rand.h>   //generate arrays of random bytes

#ifndef ASYMMETRIC_CRYPTO_H
#define ASYMMETRIC_CRYPTO_H

constexpr auto RSA_KEY_LEN = 2048;
constexpr auto PUBLIC_KEYS_AMOUNT = 1;

class AsymmetricCrypto
{
public:
    //generate keypair to encrypt or decrypt first
    AsymmetricCrypto() = default;
    ~AsymmetricCrypto();

    int Encrypt(
        const unsigned char* msg,
        unsigned int msg_len,
        std::shared_ptr<unsigned char[]>& encr_msg,
        std::shared_ptr<unsigned char[]>& encr_key,
        unsigned int* encr_key_len,
        std::shared_ptr<unsigned char[]>& iv,
        unsigned int* iv_len);
    int Encrypt(
        const std::vector<unsigned char>& msg,
        unsigned int msg_len,
        std::vector<unsigned char>& encr_msg,
        std::vector<unsigned char>& encr_key,
        unsigned int* encr_key_len,
        std::vector<unsigned char>& iv,
        unsigned int* iv_len);
    int Decrypt(
        const unsigned char* encr_msg,
        unsigned int encr_msg_len,
        unsigned char* encr_key,
        unsigned int encr_key_len,
        unsigned char* iv,
        unsigned int iv_len,
        std::shared_ptr<unsigned char[]>& decr_msg_len);
    int Decrypt(
        const std::vector<unsigned char>& encr_msg,
        unsigned int encr_msg_len,
        std::vector<unsigned char>& decr_msg,
        const std::vector<unsigned char>& encr_key,
        unsigned int encr_key_len,
        const std::vector<unsigned char>& iv,
        unsigned int iv_len);
    bool GenerateRsaKeypair();

    void get_public_key(std::shared_ptr<unsigned char[]>& public_key);
    void get_private_key(std::shared_ptr<unsigned char[]>& private_key);

private:
    EVP_PKEY*               m_keypair = nullptr;
    EVP_CIPHER_CTX*         m_rsa_encr_ctx = nullptr;
    EVP_CIPHER_CTX*         m_rsa_decr_ctx = nullptr;

    unsigned int BioToString(
        std::shared_ptr<BIO*>& bio,
        std::shared_ptr<unsigned char[]>& str);
    void DestroyContext();
};

#endif // ASYMMETRIC_CRYPTO_H