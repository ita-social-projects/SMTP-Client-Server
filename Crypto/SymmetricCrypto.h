#pragma once

#include <openssl/pem.h>    // engine interface
#include <openssl/rand.h>   //generate arrays of random bytes

#include "ICrypto.h"
#include "..\CPPLogger\CPPLogger.h"

#ifndef SYNC_CRYPTO_H
#define SYNC_CRYPTO_H

#define KEYSIZE_16 16   // 128 bit key. 10 rounds max
#define KEYSIZE_24 24   // 192 bit key. 12 rounds max
#define KEYSIZE_32 32   // 256 bit key. 14 rounds max

#define AES_ROUNDS 5    // the more rounds get the slower encryption will work

#define IVSIZE_16 16    // for EVP_aes_256_cbc cipher type

#define PASSWORD_SIZE_16 16
#define PASSWORD_SIZE_24 24
#define PASSWORD_SIZE_32 32

#define SALT_SIZE 8

#define BLOCK_SIZE 16

#define CIPHER_INIT_ENCRYPTION_CTX 1
#define CIPHER_INIT_DECRYPTION_CTX 0
#define CIPHER_INIT_UNCHANGED_CTX -1

const unsigned char     g_key[] = "lk4587hcnew;cmdsriot7roe84dfvrer";
const unsigned int      g_key_len = KEYSIZE_32;
const unsigned char     g_iv[] = "p9e8unjxajgstddd";
const unsigned int      g_iv_len = IVSIZE_16;

class SymmetricCrypto final : public ICrypt
{
public:
    SymmetricCrypto();
    SymmetricCrypto(
        unsigned char** key,
        unsigned char key_len,
        unsigned char** iv,
        unsigned char iv_len);
    ~SymmetricCrypto();

    virtual int EncryptSymmetric(
        const unsigned char* msg,
        unsigned int msg_len,
        std::shared_ptr<unsigned char>& encr_msg) override;

    virtual int EncryptSymmetric(
        const std::vector<unsigned char>& msg,
        std::vector<unsigned char>& encr_msg) override;

    virtual int DecryptSymmetric(
        const unsigned char* encr_msg,
        unsigned int encr_msg_len,
        std::shared_ptr<unsigned char>& decr_msg) override;

    virtual int DecryptSymmetric(
        const std::vector<unsigned char>& encr_msg,
        std::vector<unsigned char>& decr_msg) override;

    virtual bool GenerateRandomKey(
        unsigned int key_len,
        unsigned int iv_len) override;
    //use password and random salt to generate key and iv
    virtual bool GenerateKeyFromPassword(
        const unsigned char* password,
        unsigned int password_len) override;

    const unsigned char* get_aes_key() const;
    bool set_aes_key(unsigned char* aes_key, const unsigned int aes_key_len);
    unsigned int get_key_size() const;
    const unsigned char* get_aes_iv() const;
    bool set_aes_iv(unsigned char* aes_iv, const unsigned int aes_iv_len);
    unsigned int get_iv_size() const;
private:
    Logger LOG;
    std::shared_ptr<EVP_CIPHER_CTX*>    m_aes_encr_ctx;
    std::shared_ptr<EVP_CIPHER_CTX*>    m_aes_decr_ctx;
    std::unique_ptr<unsigned char*>     m_aes_key;
    std::unique_ptr<unsigned char*>     m_aes_iv;
    unsigned int                        m_aes_key_len = 0;
    unsigned int                        m_aes_iv_len = 0;
    bool InitializeContext();
    void DestroyContext() const;
};

#endif // SYNC_CRYPTO_H