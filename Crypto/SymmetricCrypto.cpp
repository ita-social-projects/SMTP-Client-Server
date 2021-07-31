// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"
#include "SymmetricCrypto.h"

SymmetricCrypto::SymmetricCrypto()
{
    m_aes_encr_ctx = std::make_shared<EVP_CIPHER_CTX*>();
    m_aes_decr_ctx = std::make_shared<EVP_CIPHER_CTX*>();

    InitializeContext();
    std::unique_ptr<unsigned char[]> key(new unsigned char[g_key_len + 1]);
    unsigned char* key_ptr = key.get();
    for (int i = 0; i < g_key_len; i++)
    {
        *(key_ptr + i) = *(g_key + i);
    }
    key_ptr[g_key_len] = '\0';
    std::unique_ptr<unsigned char[]> iv(new unsigned char[g_iv_len + 1]);
    unsigned char* iv_ptr = iv.get();
    for (int i = 0; i < g_iv_len; i++)
    {
        *(iv_ptr + i) = *(g_iv + i);
    }
    iv_ptr[g_iv_len] = '\0';

    m_aes_key = std::move(key);
    m_aes_key_len = g_key_len;

    m_aes_iv = std::move(iv);
    m_aes_iv_len = g_iv_len;
}

SymmetricCrypto::SymmetricCrypto(unsigned char* key, unsigned char key_len, unsigned char* iv, unsigned char iv_len)
{
    m_aes_encr_ctx = std::make_shared<EVP_CIPHER_CTX*>();
    m_aes_decr_ctx = std::make_shared<EVP_CIPHER_CTX*>();

    if (!key || !iv)
    {
        m_aes_key.reset();
        m_aes_iv.reset();
        LOG_WARN << "get null pointer";
    }
    if (key_len != KEYSIZE_16
        && key_len != KEYSIZE_24
        && key_len != KEYSIZE_32
        && iv_len != IVSIZE_16)
    {
        m_aes_key.reset();
        m_aes_iv.reset();
        LOG_WARN << "get wrong length";
    }
    else
    {
        InitializeContext();
        if (key)
        {
            size_t smart_key_len = static_cast<size_t>(key_len) + 1;
            std::unique_ptr<unsigned char[]> smart_key(new unsigned char[smart_key_len]);
            unsigned char* smart_key_ptr = smart_key.get();
            for (int i = 0; i < key_len; i++)
            {
                *(smart_key_ptr + i) = *(key + i);
            }
            smart_key_ptr[key_len] = '\0';

            m_aes_key = std::move(smart_key);
            m_aes_key_len = key_len;
        }
        if (iv)
        {
            size_t smart_iv_len = static_cast<size_t>(key_len) + 1;
            std::unique_ptr<unsigned char[]> smart_iv(new unsigned char[smart_iv_len]);
            unsigned char* smart_iv_ptr = smart_iv.get();
            for (int i = 0; i < iv_len; i++)
            {
                *(smart_iv_ptr + i) = *(iv + i);
            }
            smart_iv_ptr[iv_len] = '\0';

            m_aes_iv = std::move(smart_iv);
            m_aes_iv_len = iv_len;
        }
    }
}

bool SymmetricCrypto::GenerateRandomKey(unsigned int key_len, unsigned int iv_len)
{
    if (key_len != KEYSIZE_16
        && key_len != KEYSIZE_24
        && key_len != KEYSIZE_32
        && iv_len != IVSIZE_16)
    {
        LOG_ERROR << "get wrong length";
        return false;
    }
    std::unique_ptr<unsigned char[]> aes_key = std::make_unique<unsigned char[]>(static_cast<size_t>(key_len));
    std::unique_ptr<unsigned char[]> aes_iv = std::make_unique<unsigned char[]>(static_cast<size_t>(iv_len));

    if (RAND_bytes(aes_key.get(), (int)key_len) == 0)
    {
        LOG_ERROR << "generate rand bytes for key failed";
        return false;
    }
    if (RAND_bytes(aes_iv.get(), (int)iv_len) == 0)
    {
        LOG_ERROR << "generate rand bytes for iv failed";
        return false;
    }

    m_aes_key.reset();
    m_aes_key = std::move(aes_key);
    m_aes_key_len = key_len;

    m_aes_iv.reset();
    m_aes_iv = std::move(aes_iv);
    m_aes_iv_len = iv_len;

    return true;
}

SymmetricCrypto::~SymmetricCrypto()
{
    DestroyContext();
}

bool SymmetricCrypto::InitializeContext()
{
    *m_aes_encr_ctx.get() = EVP_CIPHER_CTX_new();
    *m_aes_decr_ctx.get() = EVP_CIPHER_CTX_new();

    if (m_aes_encr_ctx == nullptr || m_aes_decr_ctx == nullptr)
    {
        LOG_ERROR << "contexts fail to initialize";
        return false;
    }

    EVP_CIPHER_CTX_init(*m_aes_encr_ctx.get());
    EVP_CIPHER_CTX_init(*m_aes_decr_ctx.get());

    EVP_CipherInit_ex(
        *m_aes_encr_ctx.get(),
        EVP_aes_256_cbc(),
        nullptr,
        nullptr,
        nullptr,
        CIPHER_INIT_ENCRYPTION_CTX);

    m_aes_key_len = EVP_CIPHER_CTX_key_length(*m_aes_encr_ctx.get());
    m_aes_iv_len = EVP_CIPHER_CTX_iv_length(*m_aes_encr_ctx.get());

    return true;
}

void SymmetricCrypto::DestroyContext() const
{
    EVP_CIPHER_CTX_free(*m_aes_encr_ctx.get());
    EVP_CIPHER_CTX_free(*m_aes_decr_ctx.get());
}

int SymmetricCrypto::Encrypt(
    const unsigned char* msg,
    unsigned int msg_len,
    std::shared_ptr<unsigned char[]>& encr_msg)
{
    if (!msg || msg_len <= 0)
    {
        LOG_ERROR << "incorrect message";
        return -1;
    }

    int block_len = 0;
    int encr_msg_len = 0;

    size_t msg_len_plus_block_size = static_cast<size_t>(msg_len) + static_cast<size_t>(BLOCK_SIZE);
    std::shared_ptr<unsigned char[]> buff_msg(new unsigned char[msg_len_plus_block_size]);
    encr_msg = buff_msg;

    //
    if (!EVP_EncryptInit_ex(*m_aes_encr_ctx.get(), EVP_aes_256_cbc(), nullptr, m_aes_key.get(), m_aes_iv.get()))
    {
        LOG_ERROR << "EVP Encrypt Init fail";
        return -1;
    }

    if (!EVP_EncryptUpdate(*m_aes_encr_ctx.get(), encr_msg.get(), &block_len, msg, (int)msg_len))
    {
        LOG_ERROR << "EVP Encrypt Update fail";
        return -1;
    }

    encr_msg_len += block_len;

    unsigned char* encr_msg_ptr = encr_msg.get() + ptrdiff_t(encr_msg_len);
    if (!EVP_EncryptFinal_ex(*m_aes_encr_ctx.get(), encr_msg_ptr, &block_len))
    {
        LOG_ERROR << "EVP Encrypt Final fail";
        return -1;
    }

    encr_msg_len += block_len;

    unsigned char* encr_msg_end_ptr = encr_msg.get();
    size_t arr_end = ptrdiff_t(encr_msg_len);
    encr_msg_end_ptr[arr_end] = '\0';

    LOG_INFO << "Encryption succsess";
    return encr_msg_len;
}

int SymmetricCrypto::Encrypt(
    const std::vector<unsigned char>& msg,
    std::vector<unsigned char>& encr_msg)
{
    if (msg.empty())
    {
        LOG_ERROR << "message is empty";
        return -1;
    }

    int block_len = 0;
    int encr_msg_len = 0;

    encr_msg.resize(msg.size() + BLOCK_SIZE);

    if (!EVP_EncryptInit_ex(*m_aes_encr_ctx.get(), EVP_aes_256_cbc(), nullptr, m_aes_key.get(), m_aes_iv.get()))
    {
        LOG_ERROR << "EVP Encrypt Init fail";
        return -1;
    }

    if (!EVP_EncryptUpdate(
        *m_aes_encr_ctx.get(),
        &encr_msg.front(),
        &block_len,
        &msg.front(),
        (int)msg.size()))
    {
        LOG_ERROR << "EVP Encrypt Update fail";
        return -1;
    }

    encr_msg_len += block_len;

    if (!EVP_EncryptFinal_ex(*m_aes_encr_ctx.get(), &encr_msg.front() + encr_msg_len, &block_len))
    {
        LOG_ERROR << "EVP Encrypt Final fail";
        return -1;
    }

    encr_msg_len += block_len;

    LOG_INFO << "Encryption succsess";
    return encr_msg_len;
}

int SymmetricCrypto::Decrypt(
    const unsigned char* encr_msg,
    unsigned int encr_msg_len,
    std::shared_ptr<unsigned char[]>& decr_msg)
{
    if (!encr_msg || !encr_msg_len)
    {
        LOG_INFO << "enctypted message is empty";
        return -1;
    }
    int decr_msg_len = 0;
    int block_len = 0;

    size_t encr_len = ptrdiff_t(encr_msg_len);
    std::shared_ptr<unsigned char[]> msg(new unsigned char[encr_len]);
    decr_msg = msg;

    if (!EVP_DecryptInit_ex(*m_aes_decr_ctx.get(), EVP_aes_256_cbc(), nullptr, m_aes_key.get(), m_aes_iv.get()))
    {
        LOG_INFO << "EVP Decrypt Init fail";
        return -1;
    }

    if (!EVP_DecryptUpdate(*m_aes_decr_ctx.get(), decr_msg.get(), &block_len, encr_msg, (int)encr_msg_len))
    {
        LOG_INFO << "EVP Decrypt Update fail";
        return -1;
    }

    decr_msg_len += block_len;

    unsigned char* msg_end_ptr = decr_msg.get() + ptrdiff_t(decr_msg_len);
    if (!EVP_DecryptFinal_ex(*m_aes_decr_ctx.get(), msg_end_ptr, &block_len))
    {
        LOG_INFO << "EVP Decrypt Final fail";
        return -1;
    }

    decr_msg_len += block_len;

    unsigned char* decr_ptr = decr_msg.get();
    size_t arr_end = ptrdiff_t(decr_msg_len);
    decr_ptr[arr_end] = '\0';

    LOG_INFO << "Decryption succsess";
    return decr_msg_len;
}

int SymmetricCrypto::Decrypt(
    const std::vector<unsigned char>& encr_msg,
    std::vector<unsigned char>& decr_msg)
{
    if (encr_msg.empty())
    {
        LOG_INFO << "enctypted message is empty";
        return -1;
    }
    int decr_msg_len = 0;
    int block_len = 0;

    decr_msg.resize(encr_msg.size());

    if (!EVP_DecryptInit_ex(*m_aes_decr_ctx.get(), EVP_aes_256_cbc(), nullptr, m_aes_key.get(), m_aes_iv.get()))
    {
        LOG_INFO << "EVP Decrypt Init fail";
        return -1;
    }

    if (!EVP_DecryptUpdate(
        *m_aes_decr_ctx.get(),
        &decr_msg.front(),
        &block_len,
        &encr_msg.front(),
        (int)(decr_msg.size())))
    {
        LOG_INFO << "EVP Decrypt Update fail";
        return -1;
    }

    decr_msg_len += block_len;

    if (!EVP_DecryptFinal_ex(*m_aes_decr_ctx.get(), &decr_msg.front() + decr_msg_len, &block_len))
    {
        LOG_INFO << "EVP Decrypt Final fail";
        return -1;
    }

    decr_msg_len += block_len;

    LOG_INFO << "Decryption succsess";
    return decr_msg_len;
}

const unsigned char* SymmetricCrypto::get_aes_key() const
{
    return m_aes_key.get();
}

bool SymmetricCrypto::set_aes_key(unsigned char* aes_key, unsigned int aes_key_len)
{
    if (aes_key_len != m_aes_key_len)
    {
        LOG_INFO << "wrong key length";
        return false;
    }
    size_t smart_key_len = static_cast<size_t>(aes_key_len) + 1;
    std::unique_ptr<unsigned char[]> smart_key(new unsigned char[smart_key_len]);
    unsigned char* key_ptr = smart_key.get();
    m_aes_key.reset();
    for (size_t i = 0; i < static_cast<size_t>(aes_key_len); i++) {
        *(key_ptr + i) = *(aes_key + i);
    }
    key_ptr[static_cast<size_t>(aes_key_len)] = '\0';
    m_aes_key = std::move(smart_key);
    return true;
}

unsigned int SymmetricCrypto::get_key_size() const
{
    return m_aes_key_len;
}

const unsigned char* SymmetricCrypto::get_aes_iv() const
{
    return m_aes_iv.get();
}

bool SymmetricCrypto::set_aes_iv(unsigned char* aes_iv, unsigned int aes_iv_len)
{
    if (aes_iv_len != m_aes_iv_len)
    {
        LOG_INFO << "wrong iv length";
        return false;
    }
    size_t smart_iv_len = static_cast<size_t>(aes_iv_len) + 1;
    std::unique_ptr<unsigned char[]> smart_iv(new unsigned char[smart_iv_len]);
    unsigned char* key_ptr = smart_iv.get();
    m_aes_iv.reset();
    for (size_t i = 0; i < static_cast<size_t>(aes_iv_len); i++) {
        *(key_ptr + i) = *(aes_iv + i);
    }
    key_ptr[static_cast<size_t>(aes_iv_len)] = '\0';
    m_aes_iv = std::move(smart_iv);

    return true;
}

unsigned int SymmetricCrypto::get_iv_size() const
{
    return m_aes_iv_len;
}