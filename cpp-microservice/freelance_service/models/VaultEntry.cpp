#include "VaultEntry.h"

VaultEntry::VaultEntry(const std::string& uuid, const std::string& encryptedData, const std::string& createdAt)
    : uuid(uuid), encryptedData(encryptedData), createdAt(createdAt) {}
