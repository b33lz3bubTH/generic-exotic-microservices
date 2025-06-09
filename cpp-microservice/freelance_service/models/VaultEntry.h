#pragma once
#include <string>

class VaultEntry {
public:
    std::string uuid;
    std::string encryptedData;
    std::string createdAt;

    VaultEntry() = default;
    VaultEntry(const std::string& uuid, const std::string& encryptedData, const std::string& createdAt);
};
