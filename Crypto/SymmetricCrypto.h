#pragma once

#include <openssl/pem.h>            // engine interface
#include <openssl/rand.h>           //generate arrays of random bytes

#include "ICrypto.h"

#ifndef SYMMETRIC_CRYPTO_H
#define SYMMETRIC_CRYPTO_H

constexpr auto KEYSIZE_16 = 16;     // 128 bit key. 10 rounds max
constexpr auto KEYSIZE_24 = 24;     // 192 bit key. 12 rounds max
constexpr auto KEYSIZE_32 = 32;     // 256 bit key. 14 rounds max

constexpr auto AES_ROUNDS = 5;      // the more rounds get the slower encryption will work

constexpr auto IVSIZE_16 = 16;      // for EVP_aes_256_cbc cipher type

constexpr auto BLOCK_SIZE = 16;

constexpr auto SALT_SIZE = 8;

constexpr auto PASSWORD_MIN_SIZE = 8;
constexpr auto PASSWORD_MAX_SIZE = 32;

constexpr auto CIPHER_INIT_ENCRYPTION_CTX = 1;
constexpr auto CIPHER_INIT_DECRYPTION_CTX = 0;
constexpr auto CIPHER_INIT_UNCHANGED_CTX = -1;

const unsigned char     g_key[] = "lk4587hcnew;cmdsriot7roe84dfvrer";
const unsigned int      g_key_len = KEYSIZE_32;
const unsigned char     g_iv[] = "p9e8unjxajgstddd";
const unsigned int      g_iv_len = IVSIZE_16;

class SymmetricCrypto final : public ICrypt
{
public:
    SymmetricCrypto();
    SymmetricCrypto(
        unsigned char* key,
        unsigned char key_len,
        unsigned char* iv,
        unsigned char iv_len);
    ~SymmetricCrypto();

    virtual int Encrypt(
        const unsigned char* msg,
        unsigned int msg_len,
        std::shared_ptr<unsigned char[]>& encr_msg) override;

    virtual int Encrypt(
        const std::vector<unsigned char>& msg,
        std::vector<unsigned char>& encr_msg) override;

    virtual int Decrypt(
        const unsigned char* encr_msg,
        unsigned int encr_msg_len,
        std::shared_ptr<unsigned char[]>& decr_msg) override;

    virtual int Decrypt(
        const std::vector<unsigned char>& encr_msg,
        std::vector<unsigned char>& decr_msg) override;

    virtual bool GenerateRandomKey(
        unsigned int key_len,
        unsigned int iv_len) override;

    virtual bool GenerateKeyFromPassword(
        const unsigned char* password,
        unsigned int password_len) override;

    unsigned char* get_aes_key() const;
    bool set_aes_key(unsigned char* aes_key, const unsigned int aes_key_len);
    unsigned int get_key_size() const;
    unsigned char* get_aes_iv() const;
    bool set_aes_iv(unsigned char* aes_iv, const unsigned int aes_iv_len);
    unsigned int get_iv_size() const;
private:
    EVP_CIPHER_CTX*                     m_aes_encr_ctx = nullptr;
    EVP_CIPHER_CTX*                     m_aes_decr_ctx = nullptr;
    std::unique_ptr<unsigned char[]>    m_aes_key;
    std::unique_ptr<unsigned char[]>    m_aes_iv;
    unsigned int                        m_aes_key_len = 0;
    unsigned int                        m_aes_iv_len = 0;
    void DestroyContext() const;
    void InitializeDefaultKeyAndIv();
    bool InitializeEncryptContext();
    bool InitializeDecryptContext();
};

#endif // SYMMETRIC_CRYPTO_H