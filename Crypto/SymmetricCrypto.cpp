// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"
#include "SymmetricCrypto.h"

SymmetricCrypto::SymmetricCrypto()
{
    InitializeDefaultKeyAndIv();
}

SymmetricCrypto::SymmetricCrypto(unsigned char* key, unsigned char key_len, unsigned char* iv, unsigned char iv_len)
{
    if (!key || !iv)
    {
        m_aes_key.reset();
        m_aes_iv.reset();
    }
    else if (key_len != KEYSIZE_16
        && key_len != KEYSIZE_24
        && key_len != KEYSIZE_32)
    {
        m_aes_key.reset();
        m_aes_iv.reset();
    }
    else if (iv_len != IVSIZE_16)
    {
        m_aes_key.reset();
        m_aes_iv.reset();
    }
    else
    {
        size_t smart_key_len = static_cast<size_t>(key_len) + 1;
        std::unique_ptr<unsigned char[]> smart_key = std::make_unique<unsigned char[]>(smart_key_len);
        unsigned char* smart_key_ptr = smart_key.get();

        for (int i = 0; i < key_len; i++) {
            *(smart_key_ptr + i) = *(key + i);
        }
        smart_key_ptr[key_len] = '\0';

        m_aes_key = std::move(smart_key);
        m_aes_key_len = key_len;

        size_t smart_iv_len = static_cast<size_t>(key_len) + 1;
        std::unique_ptr<unsigned char[]> smart_iv = std::make_unique<unsigned char[]>(smart_iv_len);
        unsigned char* smart_iv_ptr = smart_iv.get();

        for (int i = 0; i < iv_len; i++) {
            *(smart_iv_ptr + i) = *(iv + i);
        }
        smart_iv_ptr[iv_len] = '\0';

        m_aes_iv = std::move(smart_iv);
        m_aes_iv_len = iv_len;
    }
}

bool SymmetricCrypto::GenerateRandomKey(unsigned int key_len, unsigned int iv_len)
{
    if (key_len != KEYSIZE_16
        && key_len != KEYSIZE_24
        && key_len != KEYSIZE_32)
    {
        return false;
    }
    if (iv_len != IVSIZE_16)
    {
        return false;
    }
    std::unique_ptr<unsigned char[]> aes_key = std::make_unique<unsigned char[]>
        (static_cast<size_t>(static_cast<size_t>(key_len) + 1));
    std::unique_ptr<unsigned char[]> aes_iv = std::make_unique<unsigned char[]>
        (static_cast<size_t>(static_cast<size_t>(iv_len) + 1));

    if (RAND_bytes(aes_key.get(), (int)key_len) == 0)
    {
        return false;
    }
    unsigned char* aes_key_ptr = aes_key.get();
    aes_key_ptr[static_cast<size_t>(key_len)] = '\0';

    if (RAND_bytes(aes_iv.get(), (int)iv_len) == 0)
    {
        return false;
    }
    unsigned char* aes_iv_ptr = aes_iv.get();
    aes_iv_ptr[static_cast<size_t>(iv_len)] = '\0';

    m_aes_key.reset();
    m_aes_key = std::move(aes_key);
    m_aes_key_len = key_len;

    m_aes_iv.reset();
    m_aes_iv = std::move(aes_iv);
    m_aes_iv_len = iv_len;

    return true;
}

bool SymmetricCrypto::GenerateKeyFromPassword(const unsigned char* password, unsigned int password_len)
{
    if (!password)
    {
        return false;
    }
    if (password_len < PASSWORD_MIN_SIZE
        || password_len > PASSWORD_MAX_SIZE)
    {
        return false;
    }
    std::unique_ptr<unsigned char[]> aes_salt(std::make_unique<unsigned char[]>(SALT_SIZE));
    unsigned char* aes_salt_ptr = aes_salt.get();

    if (RAND_bytes(aes_salt_ptr, SALT_SIZE) == 0)
    {
        return false;
    }

    std::unique_ptr<unsigned char[]> aes_key(std::make_unique<unsigned char[]>(KEYSIZE_32));
    std::unique_ptr<unsigned char[]> aes_iv(std::make_unique<unsigned char[]>(IVSIZE_16));
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
        return false;
    }

    m_aes_key.reset();
    m_aes_key = std::move(aes_key);
    m_aes_key_len = KEYSIZE_32;

    m_aes_iv.reset();
    m_aes_iv = std::move(aes_iv);
    m_aes_iv_len = IVSIZE_16;

    return true;
}

SymmetricCrypto::~SymmetricCrypto()
{
    DestroyContext();
}

void SymmetricCrypto::DestroyContext() const
{
    if (m_aes_encr_ctx) {
        EVP_CIPHER_CTX_free(m_aes_encr_ctx);
    }
    if (m_aes_decr_ctx) {
        EVP_CIPHER_CTX_free(m_aes_decr_ctx);
    }
}

void SymmetricCrypto::InitializeDefaultKeyAndIv()
{
    std::unique_ptr<unsigned char[]> key = std::make_unique<unsigned char[]>(g_key_len + 1);
    unsigned char* key_ptr = key.get();
    for (int i = 0; i < g_key_len; i++) {
        *(key_ptr + i) = *(g_key + i);
    }
    key_ptr[g_key_len] = '\0';

    std::unique_ptr<unsigned char[]> iv = std::make_unique<unsigned char[]>(g_iv_len + 1);
    unsigned char* iv_ptr = iv.get();
    for (int i = 0; i < g_iv_len; i++) {
        *(iv_ptr + i) = *(g_iv + i);
    }
    iv_ptr[g_iv_len] = '\0';

    m_aes_key.reset();
    m_aes_key = std::move(key);
    m_aes_key_len = g_key_len;

    m_aes_iv.reset();
    m_aes_iv = std::move(iv);
    m_aes_iv_len = g_iv_len;
}

bool SymmetricCrypto::InitializeEncryptContext()
{
    m_aes_encr_ctx = EVP_CIPHER_CTX_new();
    if (m_aes_encr_ctx == nullptr) {
        EVP_CIPHER_CTX_free(m_aes_encr_ctx);
        return false;
    }
    EVP_CIPHER_CTX_init(m_aes_encr_ctx);
    if (!EVP_CipherInit_ex(
        m_aes_encr_ctx,
        EVP_aes_256_cbc(),
        nullptr,
        nullptr,
        nullptr,
        CIPHER_INIT_ENCRYPTION_CTX))
    {
        DestroyContext();
        return false;
    }

    return true;
}

bool SymmetricCrypto::InitializeDecryptContext()
{
    m_aes_decr_ctx = EVP_CIPHER_CTX_new();
    if (m_aes_decr_ctx == nullptr) {
        DestroyContext();
        return false;
    }

    EVP_CIPHER_CTX_init(m_aes_decr_ctx);

    return true;
}

int SymmetricCrypto::Encrypt(
    const unsigned char* msg,
    unsigned int msg_len,
    std::shared_ptr<unsigned char[]>& encr_msg)
{
    if (!msg) { return (int)SymmetricErrors::E_EMPTY_MESSAGE; }

    if (msg_len <= 0) { return (int)SymmetricErrors::E_INCORRECT_LENGTH; }

    int block_len = 0;
    int encr_msg_len = 0;

    size_t msg_len_plus_block_size = static_cast<size_t>(msg_len) + static_cast<size_t>(BLOCK_SIZE);
    std::shared_ptr<unsigned char[]> buff_msg(new unsigned char[msg_len_plus_block_size]);
    encr_msg = buff_msg;

    if (!m_aes_encr_ctx) {
        if (!InitializeEncryptContext()) {
            return (int)SymmetricErrors::E_ENCRYPT_INITIALIZE_FAIL;
        }
    }

    if (!EVP_EncryptInit_ex(m_aes_encr_ctx, EVP_aes_256_cbc(), nullptr, m_aes_key.get(), m_aes_iv.get())) {
        return (int)SymmetricErrors::E_ENCRYPT_INITIALIZE_FAIL;
    }

    if (!EVP_EncryptUpdate(m_aes_encr_ctx, encr_msg.get(), &block_len, msg, (int)msg_len)) {
        return (int)SymmetricErrors::E_ENCRYPT_UPDATE_FAIL;
    }

    encr_msg_len += block_len;

    unsigned char* encr_msg_ptr = encr_msg.get() + ptrdiff_t(encr_msg_len);
    if (!EVP_EncryptFinal_ex(m_aes_encr_ctx, encr_msg_ptr, &block_len)) {
        return (int)SymmetricErrors::E_ENCRYPT_FINAL_FAIL;
    }

    encr_msg_len += block_len;   

    return encr_msg_len;
}

int SymmetricCrypto::Encrypt(
    const std::vector<unsigned char>& msg,
    std::vector<unsigned char>& encr_msg)
{
    if (msg.empty()) {
        return (int)SymmetricErrors::E_EMPTY_MESSAGE;
    }

    int block_len = 0;
    int encr_msg_len = 0;

    encr_msg.resize(msg.size() + BLOCK_SIZE);

    if (!m_aes_encr_ctx) {
        if (!InitializeEncryptContext()) {
            return (int)SymmetricErrors::E_ENCRYPT_INITIALIZE_FAIL;
        }
    }

    if (!EVP_EncryptInit_ex(m_aes_encr_ctx, EVP_aes_256_cbc(), nullptr, m_aes_key.get(), m_aes_iv.get())) {
        return (int)SymmetricErrors::E_ENCRYPT_INITIALIZE_FAIL;
    }

    if (!EVP_EncryptUpdate(
        m_aes_encr_ctx,
        &encr_msg.front(),
        &block_len,
        &msg.front(),
        (int)msg.size()))
    {
        return (int)SymmetricErrors::E_ENCRYPT_UPDATE_FAIL;
    }

    encr_msg_len += block_len;

    if (!EVP_EncryptFinal_ex(m_aes_encr_ctx, &encr_msg.front() + encr_msg_len, &block_len)) {
        return (int)SymmetricErrors::E_ENCRYPT_FINAL_FAIL;
    }

    encr_msg_len += block_len;

    return encr_msg_len;
}

int SymmetricCrypto::Decrypt(
    const unsigned char* encr_msg,
    unsigned int encr_msg_len,
    std::shared_ptr<unsigned char[]>& decr_msg)
{
    if (!encr_msg) {
        return (int)SymmetricErrors::E_EMPTY_MESSAGE;
    }
    if (!encr_msg_len) {
        return (int)SymmetricErrors::E_INCORRECT_LENGTH;
    }
    int decr_msg_len = 0;
    int block_len = 0;

    size_t encr_len = ptrdiff_t(encr_msg_len);
    std::shared_ptr<unsigned char[]> msg(new unsigned char[encr_len]);
    decr_msg = msg;

    if (!m_aes_decr_ctx) {
        if (!InitializeDecryptContext()) {
            return (int)SymmetricErrors::E_DECRYPT_INITIALIZE_FAIL;
        }
    }

    if (!EVP_DecryptInit_ex(m_aes_decr_ctx, EVP_aes_256_cbc(), nullptr, m_aes_key.get(), m_aes_iv.get())) {
        return (int)SymmetricErrors::E_DECRYPT_INITIALIZE_FAIL;
    }

    if (!EVP_DecryptUpdate(m_aes_decr_ctx, decr_msg.get(), &block_len, encr_msg, (int)encr_msg_len)) {
        return (int)SymmetricErrors::E_DECRYPT_UPDATE_FAIL;
    }

    decr_msg_len += block_len;

    unsigned char* msg_end_ptr = decr_msg.get() + ptrdiff_t(decr_msg_len);
    if (!EVP_DecryptFinal_ex(m_aes_decr_ctx, msg_end_ptr, &block_len)) {
        return (int)SymmetricErrors::E_DECRYPT_FINAL_FAIL;
    }

    decr_msg_len += block_len;

    return decr_msg_len;
}

int SymmetricCrypto::Decrypt(
    const std::vector<unsigned char>& encr_msg,
    std::vector<unsigned char>& decr_msg)
{
    if (encr_msg.empty()) {
        return (int)SymmetricErrors::E_EMPTY_MESSAGE;
    }
    int decr_msg_len = 0;
    int block_len = 0;

    decr_msg.resize(encr_msg.size());

    if (!m_aes_decr_ctx) {
        if (!InitializeDecryptContext()) {
            return (int)SymmetricErrors::E_DECRYPT_INITIALIZE_FAIL;
        }
    }

    if (!EVP_DecryptInit_ex(m_aes_decr_ctx, EVP_aes_256_cbc(), nullptr, m_aes_key.get(), m_aes_iv.get())) {
        return (int)SymmetricErrors::E_DECRYPT_INITIALIZE_FAIL;
    }

    if (!EVP_DecryptUpdate(
        m_aes_decr_ctx,
        &decr_msg.front(),
        &block_len,
        &encr_msg.front(),
        (int)(decr_msg.size())))
    {
        return (int)SymmetricErrors::E_DECRYPT_UPDATE_FAIL;
    }

    decr_msg_len += block_len;

    if (!EVP_DecryptFinal_ex(m_aes_decr_ctx, &decr_msg.front() + decr_msg_len, &block_len)) {
        return (int)SymmetricErrors::E_DECRYPT_FINAL_FAIL;
    }

    decr_msg_len += block_len;

    return decr_msg_len;
}

unsigned char* SymmetricCrypto::get_aes_key() const
{
    return m_aes_key.get();
}

bool SymmetricCrypto::set_aes_key(unsigned char* aes_key, unsigned int aes_key_len)
{
    if (aes_key_len != KEYSIZE_16
        && aes_key_len != KEYSIZE_24
        && aes_key_len != KEYSIZE_32)
    {
        return false;
    }
    size_t smart_key_len = static_cast<size_t>(aes_key_len) + 1;
    std::unique_ptr<unsigned char[]> smart_key = std::make_unique<unsigned char[]>(smart_key_len);
    unsigned char* key_ptr = smart_key.get();

    for (size_t i = 0; i < static_cast<size_t>(aes_key_len); i++) {
        *(key_ptr + i) = *(aes_key + i);
    }
    key_ptr[static_cast<size_t>(aes_key_len)] = '\0';

    m_aes_key.reset();
    m_aes_key = std::move(smart_key);

    return true;
}

unsigned int SymmetricCrypto::get_key_size() const
{
    return m_aes_key_len;
}

unsigned char* SymmetricCrypto::get_aes_iv() const
{
    return m_aes_iv.get();
}

bool SymmetricCrypto::set_aes_iv(unsigned char* aes_iv, unsigned int aes_iv_len)
{
    if (aes_iv_len != IVSIZE_16)
    {
        return false;
    }
    size_t smart_iv_len = static_cast<size_t>(aes_iv_len) + 1;
    std::unique_ptr<unsigned char[]> smart_iv = std::make_unique<unsigned char[]>(smart_iv_len);
    unsigned char* key_ptr = smart_iv.get();

    for (size_t i = 0; i < static_cast<size_t>(aes_iv_len); i++) {
        *(key_ptr + i) = *(aes_iv + i);
    }
    key_ptr[static_cast<size_t>(aes_iv_len)] = '\0';

    m_aes_iv.reset();
    m_aes_iv = std::move(smart_iv);

    return true;
}

unsigned int SymmetricCrypto::get_iv_size() const
{
    return m_aes_iv_len;
}