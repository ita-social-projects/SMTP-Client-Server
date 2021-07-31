// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"
#include "AsymmetricCrypto.h"

AsymmetricCrypto::AsymmetricCrypto()
{
    m_keypair = std::make_shared<EVP_PKEY*>();
    m_rsa_encr_ctx = std::make_shared<EVP_CIPHER_CTX*>();
    m_rsa_decr_ctx = std::make_shared<EVP_CIPHER_CTX*>();

    InitializeContext();

    GenerateRsaKeypair();
}

AsymmetricCrypto::~AsymmetricCrypto()
{
    DestroyContext();
}

bool AsymmetricCrypto::InitializeContext()
{
    *m_rsa_encr_ctx.get() = EVP_CIPHER_CTX_new();
    *m_rsa_decr_ctx.get() = EVP_CIPHER_CTX_new();

    if (!*m_rsa_encr_ctx.get() || !*m_rsa_decr_ctx.get()) {
        return false;
    }
    return true;
}

void AsymmetricCrypto::DestroyContext()
{
    EVP_PKEY_free(*m_keypair.get());

    EVP_CIPHER_CTX_free(*m_rsa_encr_ctx.get());
    EVP_CIPHER_CTX_free(*m_rsa_decr_ctx.get());
}

bool AsymmetricCrypto::GenerateRsaKeypair()
{
    std::unique_ptr<EVP_PKEY_CTX*> ctx = std::make_unique<EVP_PKEY_CTX*>();

    *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);

    if (EVP_PKEY_keygen_init(*ctx.get()) <= 0) {
        return false;
    }
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(*ctx.get(), RSA_KEY_LEN) <= 0) {
        return false;
    }
    if (EVP_PKEY_keygen(*ctx.get(), m_keypair.get()) <= 0) {
        return false;
    }

    EVP_PKEY_CTX_free(*ctx.get());

    return true;
}

int AsymmetricCrypto::Encrypt(
    const unsigned char* msg,
    unsigned int msg_len,
    std::shared_ptr<unsigned char[]>& encr_msg,
    std::shared_ptr<unsigned char[]>& encr_key,
    unsigned int* encr_key_len,
    std::shared_ptr<unsigned char[]>& iv,
    unsigned int* iv_len)
{
    if (!msg || !msg_len)
    {
        return -1;
    }

    int encr_msg_len = 0;
    int block_len = 0;

    size_t key_len = static_cast<size_t>(EVP_PKEY_size(*m_keypair.get()));
    std::shared_ptr<unsigned char[]> buff_key(new unsigned char[key_len]);
    unsigned char* key_ptr = buff_key.get();
    unsigned char** key_arr_ptr = &key_ptr;
    encr_key = buff_key;

    std::shared_ptr<unsigned char[]> buff_iv(new unsigned char[EVP_MAX_IV_LENGTH]);
    iv = buff_iv;

    *iv_len = EVP_MAX_IV_LENGTH;

    size_t msg_len_plus_block_size = ptrdiff_t(msg_len) + EVP_MAX_IV_LENGTH;
    std::shared_ptr<unsigned char[]> buff_msg(new unsigned char[msg_len_plus_block_size]);
    encr_msg = buff_msg;

    if (!EVP_SealInit(
        *m_rsa_encr_ctx.get(),
        EVP_aes_256_cbc(),
        key_arr_ptr,
        (int*)encr_key_len,
        iv.get(),
        m_keypair.get(),
        PUBLIC_KEYS_AMOUNT))
    {
        return -1;
    }

    unsigned char* msg_after_init_ptr = encr_msg.get() + ptrdiff_t(encr_msg_len);
    if (!EVP_SealUpdate(*m_rsa_encr_ctx.get(), msg_after_init_ptr, &block_len, msg, (unsigned int)msg_len))
    {
        return -1;
    }

    encr_msg_len += block_len;

    unsigned char* msg_end_ptr = encr_msg.get() + ptrdiff_t(encr_msg_len);
    if (!EVP_SealFinal(*m_rsa_encr_ctx.get(), msg_end_ptr, &block_len))
    {
        return -1;
    }

    encr_msg_len += block_len;

    unsigned char* msg_final_ptr = encr_msg.get();
    size_t encr_final_len = static_cast<size_t>(encr_msg_len);
    msg_final_ptr[encr_final_len] = '\0';

    return encr_msg_len;
}

int AsymmetricCrypto::Encrypt(
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
    int encr_msg_len = 0;
    int block_len = 0;
    size_t pub_key_len = static_cast<size_t>(EVP_PKEY_size(*m_keypair.get()));
    encr_key.resize(pub_key_len);
    iv.resize(EVP_MAX_IV_LENGTH);
    *iv_len = EVP_MAX_IV_LENGTH;

    size_t msg_len_plus_block_size = ptrdiff_t(msg_len) + ptrdiff_t(EVP_MAX_IV_LENGTH);
    encr_msg.resize(msg_len_plus_block_size);

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

    unsigned char* msg_after_init_ptr = &encr_msg.front() + encr_msg_len;
    if (!EVP_SealUpdate(*m_rsa_encr_ctx.get(), msg_after_init_ptr, &block_len, &msg.front(), msg_len))
    {
        return -1;
    }

    encr_msg_len += block_len;

    unsigned char* msg_end_ptr = &encr_msg.front() + encr_msg_len;
    if (!EVP_SealFinal(*m_rsa_encr_ctx.get(), msg_end_ptr, &block_len))
    {
        return -1;
    }

    encr_msg_len += block_len;

    return encr_msg_len;
}

int AsymmetricCrypto::Decrypt(
    const unsigned char* encr_msg,
    unsigned int encr_msg_len,
    unsigned char* encr_key,
    unsigned int encr_key_len,
    unsigned char* iv,
    unsigned int iv_len,
    std::shared_ptr<unsigned char[]>& decr_msg)
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

    int decr_msg_len = 0;
    int block_len = 0;

    size_t msg_len = ptrdiff_t(encr_msg_len) + ptrdiff_t(iv_len);
    std::shared_ptr<unsigned char[]> msg(new unsigned char[msg_len]);
    decr_msg = msg;

    if (!EVP_OpenInit(*m_rsa_decr_ctx.get(), EVP_aes_256_cbc(), encr_key, encr_key_len, iv, *m_keypair.get()))
    {
        return -1;
    }

    unsigned char* decr_msg_after_init_ptr = decr_msg.get() + ptrdiff_t(decr_msg_len);
    if (!EVP_OpenUpdate(*m_rsa_decr_ctx.get(), decr_msg_after_init_ptr, &block_len, encr_msg, encr_msg_len))
    {
        return -1;
    }

    decr_msg_len += block_len;

    unsigned char* decr_msg_end_ptr = decr_msg.get() + ptrdiff_t(decr_msg_len);
    if (!EVP_OpenFinal(*m_rsa_decr_ctx.get(), decr_msg_end_ptr, &block_len))
    {
        return -1;
    }

    decr_msg_len += block_len;

    unsigned char* msg_final_end_ptr = decr_msg.get();
    size_t decr_final_len = static_cast<size_t>(decr_msg_len);
    msg_final_end_ptr[decr_final_len] = '\0';

    return decr_msg_len;
}

int AsymmetricCrypto::Decrypt(
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
    int decr_msg_len = 0;
    int block_len = 0;

    size_t decr_vec_len = ptrdiff_t(encr_msg_len) + ptrdiff_t(iv_len);
    decr_msg.resize(decr_vec_len);

    if (!EVP_OpenInit(*m_rsa_decr_ctx.get(), EVP_aes_256_cbc(), &encr_key.front(), encr_key_len, &iv.front(), *m_keypair.get()))
    {
        return -1;
    }

    unsigned char* decr_msg_after_init_ptr = &decr_msg.front() + decr_msg_len;
    if (!EVP_OpenUpdate(*m_rsa_decr_ctx.get(), decr_msg_after_init_ptr, &block_len, &encr_msg.front(), encr_msg_len))
    {
        return -1;
    }

    decr_msg_len += block_len;

    unsigned char* decr_msg_final_ptr = &decr_msg.front() + decr_msg_len;
    if (!EVP_OpenFinal(*m_rsa_decr_ctx.get(), decr_msg_final_ptr, &block_len))
    {
        return -1;
    }

    decr_msg_len += block_len;

    return decr_msg_len;
}


void AsymmetricCrypto::get_public_key(std::shared_ptr<unsigned char[]>& public_key)
{
    std::shared_ptr<BIO*> bio = std::make_shared<BIO*>();
    *bio = BIO_new(BIO_s_mem());
    PEM_write_bio_PUBKEY(*bio.get(), *m_keypair.get());
    BioToString(bio, public_key);
}

void AsymmetricCrypto::get_private_key(std::shared_ptr<unsigned char[]>& private_key)
{
    std::shared_ptr<BIO*> bio = std::make_shared<BIO*>();
    *bio = BIO_new(BIO_s_mem());
    PEM_write_bio_PrivateKey(*bio.get(), *m_keypair.get(), nullptr, nullptr, 0, 0, nullptr);
    BioToString(bio, private_key);
}

unsigned int AsymmetricCrypto::BioToString(std::shared_ptr<BIO*>& bio, std::shared_ptr<unsigned char[]>& str)
{
    unsigned int bio_len = BIO_pending(*bio.get());
    size_t buff_str_len = static_cast<size_t>(bio_len) + 1;
    std::shared_ptr<unsigned char[]> buff_str(new unsigned char[buff_str_len]);
    str = buff_str;

    BIO_read(*bio.get(), str.get(), bio_len);

    unsigned char* strPtr = str.get();
    size_t buffLen = ptrdiff_t(bio_len);
    strPtr[buffLen] = '\0';

    BIO_free_all(*bio.get());

    return bio_len;
}