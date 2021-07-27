// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"
#include "SyncCrypto.h"

SyncCrypto::SyncCrypto()
{
    m_aes_encr_ctx = std::make_shared<EVP_CIPHER_CTX*>();
    m_aes_decr_ctx = std::make_shared<EVP_CIPHER_CTX*>();

    InitializeContext();

    unsigned char* key = (unsigned char*)g_key;
    unsigned char* iv = (unsigned char*)g_iv;

    m_aes_key = std::make_unique<unsigned char*>(key);
    m_aes_key_len = g_key_len;

    m_aes_iv = std::make_unique<unsigned char*>(iv);
    m_aes_iv_len = g_iv_len;
}

SyncCrypto::SyncCrypto(unsigned char** key, unsigned char key_len, unsigned char** iv, unsigned char iv_len)
{
    m_aes_encr_ctx = std::make_shared<EVP_CIPHER_CTX*>();
    m_aes_decr_ctx = std::make_shared<EVP_CIPHER_CTX*>();

    if (!key || !iv || !*key || !*iv)
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
        if (key != nullptr)
        {
            if (!*key)
            {
                m_aes_key = std::make_unique<unsigned char*>(*key);
                m_aes_key_len = key_len;
            }
        }
        if (iv != nullptr)
        {
            if (!*iv)
            {
                m_aes_iv = std::make_unique<unsigned char*>(*iv);
                m_aes_iv_len = iv_len;
            }
        }
    }
}

bool SyncCrypto::GenerateRandomKey(unsigned int key_len, unsigned int iv_len)
{
    if (key_len != KEYSIZE_16
        && key_len != KEYSIZE_24
        && key_len != KEYSIZE_32
        && iv_len != IVSIZE_16)
    {
        LOG_ERROR << "get wrong length";
        return false;
    }

    std::unique_ptr<unsigned char> aes_key(std::make_unique<unsigned char>(key_len));
    std::unique_ptr<unsigned char> aes_iv(std::make_unique<unsigned char>(iv_len));
    unsigned char* aes_key_ptr = aes_key.get();
    unsigned char* aes_iv_ptr = aes_iv.get();

    if (RAND_bytes(aes_key_ptr, (int)key_len) == 0)
    {
        LOG_ERROR << "generate rand bytes for key failed";
        return false;
    }

    if (RAND_bytes(aes_iv_ptr, (int)iv_len) == 0)
    {
        LOG_ERROR << "generate rand bytes for iv failed";
        return false;
    }

    m_aes_key = std::make_unique<unsigned char*>(aes_key_ptr);
    m_aes_key_len = key_len;
    m_aes_iv = std::make_unique<unsigned char*>(aes_iv_ptr);
    m_aes_iv_len = iv_len;

    return true;
}

bool SyncCrypto::GenerateKeyFromPassword(const unsigned char* password, unsigned int password_len)
{
    if (!password)
    {
        LOG_ERROR << "password is nullptr";
        return false;
    }
    if (password_len != PASSWORD_SIZE_32)
    {
        LOG_ERROR << "get wrong length";
        return false;
    }

    std::unique_ptr<unsigned char> aes_salt(std::make_unique<unsigned char>(SALT_SIZE));
    unsigned char* aes_salt_ptr = aes_salt.get();

    if (RAND_bytes(aes_salt_ptr, SALT_SIZE) == 0)
    {
        LOG_ERROR << "generate rand bytes for salt failed";
        return false;
    }

    std::unique_ptr<unsigned char> aes_key(std::make_unique<unsigned char>(password_len));
    std::unique_ptr<unsigned char> aes_iv(std::make_unique<unsigned char>(IVSIZE_16));
    unsigned char* aes_key_ptr = aes_key.get();
    unsigned char* aes_iv_ptr = aes_iv.get();

    if (EVP_BytesToKey(
        EVP_aes_256_cbc(),
        EVP_sha256(),
        aes_salt_ptr,
        password,
        password_len,
        AES_ROUNDS,
        aes_key_ptr,
        aes_iv_ptr) == 0)
    {
        LOG_ERROR << "generate rand bytes for key and iv from salt and password failed";
        return false;
    }

    m_aes_key = std::make_unique<unsigned char*>(aes_key_ptr);
    m_aes_key_len = password_len;
    m_aes_iv = std::make_unique<unsigned char*>(aes_iv_ptr);
    m_aes_iv_len = IVSIZE_16;

    return true;
}

SyncCrypto::~SyncCrypto()
{
    DestroyContext();
}

bool SyncCrypto::InitializeContext()
{
    *m_aes_encr_ctx.get() = EVP_CIPHER_CTX_new();
    *m_aes_decr_ctx.get() = EVP_CIPHER_CTX_new();

    if (m_aes_encr_ctx == nullptr || m_aes_decr_ctx == nullptr) {
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

void SyncCrypto::DestroyContext() const
{
    EVP_CIPHER_CTX_free(*m_aes_encr_ctx.get());
    EVP_CIPHER_CTX_free(*m_aes_decr_ctx.get());
}

int SyncCrypto::EncryptSync(
    const unsigned char* msg,
    unsigned int msg_len,
    std::shared_ptr<unsigned char>& encr_msg)
{
    if (!msg || !msg_len)
    {
        LOG_ERROR << "incorrect message";
        return -1;
    }

    int blockLen = 0;
    int encrMsgLen = 0;

    size_t msgLen = static_cast<size_t>(msg_len) + static_cast<size_t>(BLOCK_SIZE);
    std::shared_ptr<unsigned char> buffMsg(
        new unsigned char[msgLen],
        std::default_delete<unsigned char[]>());
    encr_msg = buffMsg;

    if (!EVP_EncryptInit_ex(*m_aes_encr_ctx.get(), EVP_aes_256_cbc(), nullptr, *m_aes_key, *m_aes_iv))
    {
        LOG_ERROR << "EVP Encrypt Init fail";
        return -1;
    }

    if (!EVP_EncryptUpdate(*m_aes_encr_ctx.get(), encr_msg.get(), &blockLen, msg, msg_len))
    {
        LOG_ERROR << "EVP Encrypt Update fail";
        return -1;
    }

    encrMsgLen += blockLen;

    unsigned char* ptr = encr_msg.get() + ptrdiff_t(encrMsgLen);
    if (!EVP_EncryptFinal_ex(*m_aes_encr_ctx.get(), ptr, &blockLen))
    {
        LOG_ERROR << "EVP Encrypt Final fail";
        return -1;
    }

    encrMsgLen += blockLen;

    unsigned char* encrMsgPtr = encr_msg.get();
    size_t buff = ptrdiff_t(encrMsgLen);
    encrMsgPtr[buff] = '\0';

    LOG_INFO << "Encryption succsess";
    return encrMsgLen;
}

int SyncCrypto::EncryptSync(
    const std::vector<unsigned char>& msg,
    std::vector<unsigned char>& encr_msg)
{
    if (msg.empty())
    {
        LOG_ERROR << "message is empty";
        return -1;
    }

    int blockLen = 0;
    int encrMsgLen = 0;

    encr_msg.resize(msg.size() + BLOCK_SIZE);

    if (!EVP_EncryptInit_ex(*m_aes_encr_ctx.get(), EVP_aes_256_cbc(), nullptr, *m_aes_key, *m_aes_iv))
    {
        LOG_ERROR << "EVP Encrypt Init fail";
        return -1;
    }

    if (!EVP_EncryptUpdate(
        *m_aes_encr_ctx.get(),
        &encr_msg.front(),
        &blockLen,
        &msg.front(),
        static_cast<unsigned int>(msg.size())))
    {
        LOG_ERROR << "EVP Encrypt Update fail";
        return -1;
    }

    encrMsgLen += blockLen;

    if (!EVP_EncryptFinal_ex(*m_aes_encr_ctx.get(), &encr_msg.front() + encrMsgLen, &blockLen))
    {
        LOG_ERROR << "EVP Encrypt Final fail";
        return -1;
    }

    encrMsgLen += blockLen;

    LOG_INFO << "Encryption succsess";
    return encrMsgLen;
}

int SyncCrypto::DecryptSync(
    const unsigned char* encr_msg,
    unsigned int encr_msg_len,
    std::shared_ptr<unsigned char>& decr_msg)
{
    if (!encr_msg || !encr_msg_len)
    {
        LOG_INFO << "enctypted message is empty";
        return -1;
    }
    int decrMsgLen = 0;
    int blockLen = 0;

    size_t encrLen = ptrdiff_t(encr_msg_len);
    std::shared_ptr<unsigned char> msg(
        new unsigned char[encrLen],
        std::default_delete<unsigned char[]>());
    decr_msg = msg;

    if (!EVP_DecryptInit_ex(*m_aes_decr_ctx.get(), EVP_aes_256_cbc(), nullptr, *m_aes_key, *m_aes_iv))
    {
        LOG_INFO << "EVP Decrypt Init fail";
        return -1;
    }

    if (!EVP_DecryptUpdate(*m_aes_decr_ctx.get(), decr_msg.get(), &blockLen, encr_msg, encr_msg_len))
    {
        LOG_INFO << "EVP Decrypt Update fail";
        return -1;
    }

    decrMsgLen += blockLen;

    unsigned char* ptr = decr_msg.get() + ptrdiff_t(decrMsgLen);
    if (!EVP_DecryptFinal_ex(*m_aes_decr_ctx.get(), ptr, &blockLen))
    {
        LOG_INFO << "EVP Decrypt Final fail";
        return -1;
    }

    decrMsgLen += blockLen;

    unsigned char* decrPtr = decr_msg.get();
    size_t arrEnd = ptrdiff_t(decrMsgLen);
    decrPtr[arrEnd] = '\0';

    LOG_INFO << "Decryption succsess";
    return decrMsgLen;
}

int SyncCrypto::DecryptSync(
    const std::vector<unsigned char>& encr_msg,
    std::vector<unsigned char>& decr_msg)
{
    if (encr_msg.empty())
    {
        LOG_INFO << "enctypted message is empty";
        return -1;
    }
    int decrMsgLen = 0;
    int blockLen = 0;

    decr_msg.resize(encr_msg.size());

    if (!EVP_DecryptInit_ex(*m_aes_decr_ctx.get(), EVP_aes_256_cbc(), nullptr, *m_aes_key, *m_aes_iv))
    {
        LOG_INFO << "EVP Decrypt Init fail";
        return -1;
    }

    if (!EVP_DecryptUpdate(
        *m_aes_decr_ctx.get(),
        &decr_msg.front(),
        &blockLen,
        &encr_msg.front(),
        static_cast<unsigned int>(decr_msg.size())))
    {
        LOG_INFO << "EVP Decrypt Update fail";
        return -1;
    }

    decrMsgLen += blockLen;

    if (!EVP_DecryptFinal_ex(*m_aes_decr_ctx.get(), &decr_msg.front() + decrMsgLen, &blockLen))
    {
        LOG_INFO << "EVP Decrypt Final fail";
        return -1;
    }

    decrMsgLen += blockLen;

    LOG_INFO << "Decryption succsess";
    return decrMsgLen;
}

const unsigned char* SyncCrypto::get_aes_key() const
{
    return *m_aes_key;
}

bool SyncCrypto::set_aes_key(unsigned char* aes_key, unsigned int aes_key_len)
{
    if (aes_key_len != m_aes_key_len)
    {
        LOG_INFO << "wrong key length";
        return -1;
    }

    m_aes_key.reset();
    m_aes_key = std::make_unique<unsigned char*>(aes_key);

    return true;
}

unsigned int SyncCrypto::get_key_size() const
{
    return m_aes_key_len;
}

const unsigned char* SyncCrypto::get_aes_iv() const
{
    return *m_aes_iv;
}

bool SyncCrypto::set_aes_iv(unsigned char* aes_iv, unsigned int aes_iv_len)
{
    if (aes_iv_len != m_aes_iv_len)
    {
        LOG_INFO << "wrong iv length";
        return -1;
    }

    m_aes_iv.reset();
    m_aes_iv = std::make_unique<unsigned char*>(aes_iv);

    return true;
}

unsigned int SyncCrypto::get_iv_size() const
{
    return m_aes_iv_len;
}