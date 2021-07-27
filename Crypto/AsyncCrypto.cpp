// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"
#include "AsyncCrypto.h"

AsyncCrypto::AsyncCrypto()
{
    m_keypair = std::make_shared<EVP_PKEY*>();
    m_rsa_encr_ctx = std::make_shared<EVP_CIPHER_CTX*>();
    m_rsa_decr_ctx = std::make_shared<EVP_CIPHER_CTX*>();

    InitializeContext();

    GenerateRsaKeypair();
}

AsyncCrypto::~AsyncCrypto()
{
    DestroyContext();
}

bool AsyncCrypto::InitializeContext()
{
    *m_rsa_encr_ctx.get() = EVP_CIPHER_CTX_new();
    *m_rsa_decr_ctx.get() = EVP_CIPHER_CTX_new();

    if (!*m_rsa_encr_ctx.get() || !*m_rsa_decr_ctx.get()) {
        return false;
    }
    return true;
}

void AsyncCrypto::DestroyContext()
{
    EVP_PKEY_free(*m_keypair.get());

    EVP_CIPHER_CTX_free(*m_rsa_encr_ctx.get());
    EVP_CIPHER_CTX_free(*m_rsa_decr_ctx.get());
}

bool AsyncCrypto::GenerateRsaKeypair()
{
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);

    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        return false;
    }
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, RSA_KEY_LEN) <= 0) {
        return false;
    }
    if (EVP_PKEY_keygen(ctx, m_keypair.get()) <= 0) {
        return false;
    }

    EVP_PKEY_CTX_free(ctx);

    return true;
}

int AsyncCrypto::Encrypt(
    const unsigned char* msg,
    unsigned int msg_len,
    std::shared_ptr<unsigned char>& encr_msg,
    std::shared_ptr<unsigned char>& encr_key,
    unsigned int* encr_key_len,
    std::shared_ptr<unsigned char>& iv,
    unsigned int* iv_len)
{
    if (!msg || !msg_len)
    {
        return -1;
    }

    int encrMsgLen = 0;
    int blockLen = 0;

    size_t keyLen = static_cast<size_t>(EVP_PKEY_size(*m_keypair.get()));
    std::shared_ptr<unsigned char> buffKey(
        new unsigned char[keyLen],
        std::default_delete<unsigned char[]>());

    unsigned char* key_ptr = buffKey.get();
    unsigned char** key_ptr2 = &key_ptr;
    encr_key = buffKey;

    std::shared_ptr<unsigned char> buffIv(
        new unsigned char[EVP_MAX_IV_LENGTH],
        std::default_delete<unsigned char[]>());
    iv = buffIv;

    *iv_len = EVP_MAX_IV_LENGTH;

    size_t msgLen = ptrdiff_t(msg_len) + EVP_MAX_IV_LENGTH;
    std::shared_ptr<unsigned char> buffMsg(
        new unsigned char[msgLen],
        std::default_delete<unsigned char[]>());
    encr_msg = buffMsg;

    if (!EVP_SealInit(
        *m_rsa_encr_ctx.get(),
        EVP_aes_256_cbc(),
        key_ptr2,
        (int*)encr_key_len,
        iv.get(),
        m_keypair.get(),
        PUBLIC_KEYS_AMOUNT))
    {
        return -1;
    }

    if (!EVP_SealUpdate(*m_rsa_encr_ctx.get(), encr_msg.get() + encrMsgLen, &blockLen, msg, (unsigned int)msg_len))
    {
        return -1;
    }

    encrMsgLen += blockLen;

    unsigned char* ptr = encr_msg.get();

    unsigned char* ptr2 = ptr + ptrdiff_t(encrMsgLen);
    if (!EVP_SealFinal(*m_rsa_encr_ctx.get(), ptr2, &blockLen))
    {
        return -1;
    }

    encrMsgLen += blockLen;

    return encrMsgLen;
}

int AsyncCrypto::Encrypt(
    const std::vector<unsigned char>& msg,
    unsigned int msg_len,
    std::vector<unsigned char>& encr_msg,
    std::vector<unsigned char>& encr_key,
    unsigned int* encr_key_len,
    std::vector<unsigned char>& iv,
    unsigned int* iv_len)
{
    if (msg.empty() || !msg_len)
    {
        return -1;
    }
    int encrMsgLen = 0;
    int blockLen = 0;
    size_t pubKeyLen = static_cast<size_t>(EVP_PKEY_size(*m_keypair.get()));
    encr_key.resize(pubKeyLen);
    iv.resize(EVP_MAX_IV_LENGTH);
    *iv_len = EVP_MAX_IV_LENGTH;

    size_t buff = ptrdiff_t(msg_len) + ptrdiff_t(EVP_MAX_IV_LENGTH);
    encr_msg.resize(buff);

    unsigned char* key_ptr = &encr_key.front();
    unsigned char** key_arr_ptr = &key_ptr;

    if (!EVP_SealInit(
        *m_rsa_encr_ctx.get(),
        EVP_aes_256_cbc(),
        key_arr_ptr,
        (int*)encr_key_len,
        &iv.front(),
        m_keypair.get(),
        PUBLIC_KEYS_AMOUNT))
    {
        return -1;
    }

    if (!EVP_SealUpdate(*m_rsa_encr_ctx.get(), &encr_msg.front() + encrMsgLen, &blockLen, &msg.front(), msg_len))
    {
        return -1;
    }

    encrMsgLen += blockLen;

    if (!EVP_SealFinal(*m_rsa_encr_ctx.get(), &encr_msg.front() + encrMsgLen, &blockLen))
    {
        return -1;
    }

    encrMsgLen += blockLen;

    return encrMsgLen;
}

int AsyncCrypto::Decrypt(
    const unsigned char* encr_msg,
    unsigned int encr_msg_len,
    unsigned char* encr_key,
    unsigned int encr_key_len,
    unsigned char* iv,
    unsigned int iv_len,
    std::shared_ptr<unsigned char>& decr_msg)
{
    if (!encr_msg || !encr_msg_len)
    {
        return -1;
    }
    if (!encr_key || !encr_key_len)
    {
        return -1;
    }
    if (!iv || !iv_len)
    {
        return -1;
    }
    int decrMsgLen = 0;
    int blockLen = 0;

    std::shared_ptr<unsigned char> msg(
        new unsigned char[ptrdiff_t(encr_msg_len) + ptrdiff_t(iv_len)],
        std::default_delete<unsigned char[]>());
    decr_msg = msg;

    if (!EVP_OpenInit(*m_rsa_decr_ctx.get(), EVP_aes_256_cbc(), encr_key, encr_key_len, iv, *m_keypair.get()))
    {
        return -1;
    }

    if (!EVP_OpenUpdate(*m_rsa_decr_ctx.get(), decr_msg.get() + decrMsgLen, &blockLen, encr_msg, encr_msg_len))
    {
        return -1;
    }

    decrMsgLen += blockLen;

    unsigned char* decrMsgPtr = decr_msg.get() + ptrdiff_t(decrMsgLen);

    if (!EVP_OpenFinal(*m_rsa_decr_ctx.get(), decrMsgPtr, &blockLen))
    {
        return -1;
    }

    decrMsgLen += blockLen;

    unsigned char* msgEndPtr = decr_msg.get();
    size_t buff = ptrdiff_t(decrMsgLen);
    msgEndPtr[buff] = '\0';

    return decrMsgLen;
}

int AsyncCrypto::Decrypt(
    const std::vector<unsigned char>& encr_msg,
    unsigned int encr_msg_len,
    std::vector<unsigned char>& decr_msg,
    const std::vector<unsigned char>& encr_key,
    unsigned int encr_key_len,
    const std::vector<unsigned char>& iv,
    unsigned int iv_len)
{
    if (encr_msg.empty() || !encr_msg_len)
    {
        return -1;
    }
    if (encr_key.empty() || !encr_key_len)
    {
        return -1;
    }
    if (iv.empty() || !iv_len)
    {
        return -1;
    }
    int decrMsgLen = 0;
    int blockLen = 0;

    size_t buff = ptrdiff_t(encr_msg_len) + ptrdiff_t(iv_len);
    decr_msg.resize(buff);

    if (!EVP_OpenInit(*m_rsa_decr_ctx.get(), EVP_aes_256_cbc(), &encr_key.front(), encr_key_len, &iv.front(), *m_keypair.get()))
    {
        return -1;
    }

    if (!EVP_OpenUpdate(*m_rsa_decr_ctx.get(), &decr_msg.front() + decrMsgLen, &blockLen, &encr_msg.front(), encr_msg_len))
    {
        return -1;
    }

    decrMsgLen += blockLen;

    if (!EVP_OpenFinal(*m_rsa_decr_ctx.get(), &decr_msg.front() + decrMsgLen, &blockLen))
    {
        return -1;
    }

    decrMsgLen += blockLen;

    return decrMsgLen;
}


void AsyncCrypto::get_public_key(std::shared_ptr<unsigned char>& public_key)
{
    BIO* bio = BIO_new(BIO_s_mem());
    PEM_write_bio_PUBKEY(bio, *m_keypair.get());
    BioToString(bio, public_key);
}

void AsyncCrypto::get_private_key(std::shared_ptr<unsigned char>& private_key)
{
    BIO* bio = BIO_new(BIO_s_mem());
    PEM_write_bio_PrivateKey(bio, *m_keypair.get(), nullptr, nullptr, 0, 0, nullptr);
    BioToString(bio, private_key);
}

unsigned int AsyncCrypto::BioToString(BIO* bio, std::shared_ptr<unsigned char>& str)
{
    unsigned int bioLength = BIO_pending(bio);
    str = std::make_shared<unsigned char>(bioLength + 1);

    BIO_read(bio, str.get(), bioLength);
    if (str.get())
    {
        unsigned char* strPtr = str.get();
        size_t buffLen = ptrdiff_t(bioLength);
        strPtr[buffLen] = '\0';
    }

    BIO_free_all(bio);

    return bioLength;
}