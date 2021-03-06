// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"
#include "AsymmetricCrypto.h"

AsymmetricCrypto::~AsymmetricCrypto()
{
    DestroyContext();
}

void AsymmetricCrypto::DestroyContext()
{
    if (m_keypair) {
        EVP_PKEY_free(m_keypair);
    }
    if (m_rsa_encr_ctx) {
        EVP_CIPHER_CTX_free(m_rsa_encr_ctx);
    }
    if (m_rsa_decr_ctx) {
        EVP_CIPHER_CTX_free(m_rsa_decr_ctx);
    }
}

bool AsymmetricCrypto::GenerateRsaKeypair()
{
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);

    if (EVP_PKEY_keygen_init(ctx) <= 0
        || EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, RSA_KEY_LEN) <= 0
        || EVP_PKEY_keygen(ctx, &m_keypair) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        return false;
    }
    EVP_PKEY_CTX_free(ctx);

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
    if (!m_keypair) {
        return (int)AsymmetricErrors::E_KEYPAIR_EMPTY;
    }
    if (!msg) {
        return (int)AsymmetricErrors::E_EMPTY_MESSAGE;
    }
    if (!msg_len) {
        return (int)AsymmetricErrors::E_INCORRECT_LENGTH;
    }
    int encr_msg_len = 0;
    int block_len = 0;

    size_t key_len = static_cast<size_t>(EVP_PKEY_size(m_keypair));
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

    if (!m_rsa_encr_ctx) {
        m_rsa_encr_ctx = EVP_CIPHER_CTX_new();
    }

    if (!EVP_SealInit(
        m_rsa_encr_ctx,
        EVP_aes_256_cbc(),
        key_arr_ptr,
        (int*)encr_key_len,
        iv.get(),
        &m_keypair,
        PUBLIC_KEYS_AMOUNT))
    {
        return (int)AsymmetricErrors::E_ENCRYPT_INITIALIZE_FAIL;
    }

    unsigned char* msg_after_init_ptr = encr_msg.get() + ptrdiff_t(encr_msg_len);
    if (!EVP_SealUpdate(m_rsa_encr_ctx, msg_after_init_ptr, &block_len, msg, (unsigned int)msg_len)) {
        return (int)AsymmetricErrors::E_ENCRYPT_UPDATE_FAIL;
    }

    encr_msg_len += block_len;

    unsigned char* msg_end_ptr = encr_msg.get() + ptrdiff_t(encr_msg_len);
    if (!EVP_SealFinal(m_rsa_encr_ctx, msg_end_ptr, &block_len)) {
        return (int)AsymmetricErrors::E_ENCRYPT_FINAL_FAIL;
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
    if (!m_keypair) {
        return (int)AsymmetricErrors::E_KEYPAIR_EMPTY;
    }
    if (msg.empty()) {
        return (int)AsymmetricErrors::E_EMPTY_MESSAGE;
    }
    if (!msg_len) {
        return (int)AsymmetricErrors::E_INCORRECT_LENGTH;
    }
    int encr_msg_len = 0;
    int block_len = 0;
    size_t pub_key_len = static_cast<size_t>(EVP_PKEY_size(m_keypair));
    encr_key.resize(pub_key_len);
    iv.resize(EVP_MAX_IV_LENGTH);
    *iv_len = EVP_MAX_IV_LENGTH;

    size_t msg_len_plus_block_size = ptrdiff_t(msg_len) + ptrdiff_t(EVP_MAX_IV_LENGTH);
    encr_msg.resize(msg_len_plus_block_size);

    unsigned char* key_ptr = &encr_key.front();
    unsigned char** key_arr_ptr = &key_ptr;

    if (!m_rsa_encr_ctx) {
        m_rsa_encr_ctx = EVP_CIPHER_CTX_new();
    }

    if (!EVP_SealInit(
        m_rsa_encr_ctx,
        EVP_aes_256_cbc(),
        key_arr_ptr,
        (int*)encr_key_len,
        &iv.front(),
        &m_keypair,
        PUBLIC_KEYS_AMOUNT))
    {
        return (int)AsymmetricErrors::E_ENCRYPT_INITIALIZE_FAIL;
    }

    unsigned char* msg_after_init_ptr = &encr_msg.front() + encr_msg_len;
    if (!EVP_SealUpdate(m_rsa_encr_ctx, msg_after_init_ptr, &block_len, &msg.front(), msg_len))
    {
        return (int)AsymmetricErrors::E_ENCRYPT_UPDATE_FAIL;
    }

    encr_msg_len += block_len;

    unsigned char* msg_end_ptr = &encr_msg.front() + encr_msg_len;
    if (!EVP_SealFinal(m_rsa_encr_ctx, msg_end_ptr, &block_len))
    {
        return (int)AsymmetricErrors::E_ENCRYPT_FINAL_FAIL;
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
    if (!m_keypair) {
        return (int)AsymmetricErrors::E_KEYPAIR_EMPTY;
    }
    if (!encr_msg) {
        return (int)AsymmetricErrors::E_EMPTY_MESSAGE;
    }
    if (!encr_key) {
        return (int)AsymmetricErrors::E_EMPTY_KEY;
    }
    if (!iv) {
        return (int)AsymmetricErrors::E_EMPTY_IV;
    }
    if (!encr_msg_len || !encr_key_len || !iv_len) {
        return (int)AsymmetricErrors::E_INCORRECT_LENGTH;
    }

    int decr_msg_len = 0;
    int block_len = 0;

    size_t msg_len = ptrdiff_t(encr_msg_len) + ptrdiff_t(iv_len);
    std::shared_ptr<unsigned char[]> msg(new unsigned char[msg_len]);
    decr_msg = msg;

    if (!m_rsa_decr_ctx) {
        m_rsa_decr_ctx = EVP_CIPHER_CTX_new();
    }

    if (!EVP_OpenInit(m_rsa_decr_ctx, EVP_aes_256_cbc(), encr_key, encr_key_len, iv, m_keypair)) {
        return (int)AsymmetricErrors::E_DECRYPT_INITIALIZE_FAIL;
    }

    unsigned char* decr_msg_after_init_ptr = decr_msg.get() + ptrdiff_t(decr_msg_len);
    if (!EVP_OpenUpdate(m_rsa_decr_ctx, decr_msg_after_init_ptr, &block_len, encr_msg, encr_msg_len)) {
        return (int)AsymmetricErrors::E_DECRYPT_UPDATE_FAIL;
    }

    decr_msg_len += block_len;

    unsigned char* decr_msg_end_ptr = decr_msg.get() + ptrdiff_t(decr_msg_len);
    if (!EVP_OpenFinal(m_rsa_decr_ctx, decr_msg_end_ptr, &block_len)) {
        return (int)AsymmetricErrors::E_DECRYPT_FINAL_FAIL;
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
    if (!m_keypair) {
        return (int)AsymmetricErrors::E_KEYPAIR_EMPTY;
    }
    if (encr_msg.empty()) {
        return (int)AsymmetricErrors::E_EMPTY_MESSAGE;
    }
    if (encr_key.empty()) {
        return (int)AsymmetricErrors::E_EMPTY_KEY;
    }
    if (iv.empty()) {
        return (int)AsymmetricErrors::E_EMPTY_IV;
    }
    if (!encr_msg_len || !encr_key_len || !iv_len) {
        return (int)AsymmetricErrors::E_INCORRECT_LENGTH;
    }

    int decr_msg_len = 0;
    int block_len = 0;

    size_t decr_vec_len = ptrdiff_t(encr_msg_len) + ptrdiff_t(iv_len);
    decr_msg.resize(decr_vec_len);

    if (!m_rsa_decr_ctx) {
        m_rsa_decr_ctx = EVP_CIPHER_CTX_new();
    }

    if (!EVP_OpenInit(m_rsa_decr_ctx, EVP_aes_256_cbc(), &encr_key.front(), encr_key_len, &iv.front(), m_keypair)) {
        return (int)AsymmetricErrors::E_DECRYPT_INITIALIZE_FAIL;
    }

    unsigned char* decr_msg_after_init_ptr = &decr_msg.front() + decr_msg_len;
    if (!EVP_OpenUpdate(m_rsa_decr_ctx, decr_msg_after_init_ptr, &block_len, &encr_msg.front(), encr_msg_len)) {
        return (int)AsymmetricErrors::E_DECRYPT_UPDATE_FAIL;
    }

    decr_msg_len += block_len;

    unsigned char* decr_msg_final_ptr = &decr_msg.front() + decr_msg_len;
    if (!EVP_OpenFinal(m_rsa_decr_ctx, decr_msg_final_ptr, &block_len)) {
        return (int)AsymmetricErrors::E_DECRYPT_FINAL_FAIL;
    }

    decr_msg_len += block_len;

    return decr_msg_len;
}


void AsymmetricCrypto::get_public_key(std::shared_ptr<unsigned char[]>& public_key)
{
    std::shared_ptr<BIO*> bio = std::make_shared<BIO*>();
    *bio = BIO_new(BIO_s_mem());
    PEM_write_bio_PUBKEY(*bio, m_keypair);
    BioToString(bio, public_key);
}

void AsymmetricCrypto::get_private_key(std::shared_ptr<unsigned char[]>& private_key)
{
    std::shared_ptr<BIO*> bio = std::make_shared<BIO*>();
    *bio = BIO_new(BIO_s_mem());
    PEM_write_bio_PrivateKey(*bio, m_keypair, nullptr, nullptr, 0, 0, nullptr);
    BioToString(bio, private_key);
}

unsigned int AsymmetricCrypto::BioToString(std::shared_ptr<BIO*>& bio, std::shared_ptr<unsigned char[]>& str)
{
    unsigned int bio_len = BIO_pending(*bio);
    size_t buff_str_len = static_cast<size_t>(bio_len) + 1;
    std::shared_ptr<unsigned char[]> buff_str(new unsigned char[buff_str_len]);
    str = buff_str;

    BIO_read(*bio, str.get(), bio_len);

    unsigned char* strPtr = str.get();
    size_t buffLen = ptrdiff_t(bio_len);
    strPtr[buffLen] = '\0';

    BIO_free_all(*bio);

    return bio_len;
}