#include "VaultController.h"
#include <nlohmann/json.hpp>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>

using json = nlohmann::json;
using namespace Pistache;

namespace {
// Simple AES-256-CBC encryption/decryption helpers
std::string deriveKey(const std::string& key) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char*)key.data(), key.size(), hash);
    return std::string((char*)hash, SHA256_DIGEST_LENGTH);
}

std::string encrypt(const std::string& plaintext, const std::string& key) {
    std::string aesKey = deriveKey(key);
    unsigned char iv[16];
    RAND_bytes(iv, sizeof(iv));
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    std::string ciphertext;
    int len;
    int ciphertext_len;
    ciphertext.resize(plaintext.size() + 32);
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, (const unsigned char*)aesKey.data(), iv);
    EVP_EncryptUpdate(ctx, (unsigned char*)&ciphertext[0], &len, (const unsigned char*)plaintext.data(), plaintext.size());
    ciphertext_len = len;
    EVP_EncryptFinal_ex(ctx, (unsigned char*)&ciphertext[0] + len, &len);
    ciphertext_len += len;
    EVP_CIPHER_CTX_free(ctx);
    ciphertext.resize(ciphertext_len);
    // Prepend IV (hex) to ciphertext (hex)
    std::ostringstream oss;
    for (int i = 0; i < 16; ++i) oss << std::hex << std::setw(2) << std::setfill('0') << (int)iv[i];
    for (unsigned char c : ciphertext) oss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    return oss.str();
}

std::string decrypt(const std::string& hex, const std::string& key) {
    if (hex.size() < 32) throw std::runtime_error("Invalid encrypted data");
    std::string aesKey = deriveKey(key);
    unsigned char iv[16];
    for (int i = 0; i < 16; ++i) {
        std::string byteStr = hex.substr(i*2, 2);
        iv[i] = (unsigned char)std::stoi(byteStr, nullptr, 16);
    }
    std::string ciphertext;
    for (size_t i = 32; i < hex.size(); i += 2) {
        std::string byteStr = hex.substr(i, 2);
        ciphertext.push_back((char)std::stoi(byteStr, nullptr, 16));
    }
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    std::string plaintext;
    plaintext.resize(ciphertext.size());
    int len, plaintext_len;
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, (const unsigned char*)aesKey.data(), iv);
    EVP_DecryptUpdate(ctx, (unsigned char*)&plaintext[0], &len, (const unsigned char*)ciphertext.data(), ciphertext.size());
    plaintext_len = len;
    EVP_DecryptFinal_ex(ctx, (unsigned char*)&plaintext[0] + len, &len);
    plaintext_len += len;
    EVP_CIPHER_CTX_free(ctx);
    plaintext.resize(plaintext_len);
    return plaintext;
}
}

VaultController::VaultController(Rest::Router& router) {
    Rest::Routes::Post(router, "/vault/data", Rest::Routes::bind(&VaultController::handleCreateVault, this));
    Rest::Routes::Get(router, "/vault/data/:uuid", Rest::Routes::bind(&VaultController::handleGetVault, this));
}

void VaultController::handleCreateVault(const Rest::Request& request, Http::ResponseWriter response) {
    try {
        auto data = json::parse(request.body());
        if (!data.contains("data") || !data.contains("key")) {
            response.send(Http::Code::Bad_Request, R"({\"error\":\"Missing data or key\"})", MIME(Application, Json));
            return;
        }
        std::string plain = data["data"].dump();
        std::string key = data["key"].get<std::string>();
        std::string encrypted = encrypt(plain, key);
        std::string uuid = repository.createVaultEntry(encrypted);
        response.send(Http::Code::Created, json({{"uuid", uuid}}).dump(), MIME(Application, Json));
    } catch (const std::exception& e) {
        response.send(Http::Code::Bad_Request, json({{"error", e.what()}}).dump(), MIME(Application, Json));
    }
}

void VaultController::handleGetVault(const Rest::Request& request, Http::ResponseWriter response) {
    auto uuid = request.param(":uuid").as<std::string>();
    auto keyQ = request.query().get("key");
    if (!keyQ) {
        response.send(Http::Code::Bad_Request, R"({\"error\":\"Missing key query param\"})", MIME(Application, Json));
        return;
    }
    std::string key = *keyQ;
    auto entry = repository.getVaultEntry(uuid);
    if (!entry) {
        response.send(Http::Code::Not_Found, R"({\"error\":\"Not found\"})", MIME(Application, Json));
        return;
    }
    try {
        std::string decrypted = decrypt(entry->encryptedData, key);
        response.send(Http::Code::Ok, decrypted, MIME(Application, Json));
    } catch (const std::exception& e) {
        response.send(Http::Code::Bad_Request, json({{"error", "Invalid key or corrupt data"}}).dump(), MIME(Application, Json));
    }
}
