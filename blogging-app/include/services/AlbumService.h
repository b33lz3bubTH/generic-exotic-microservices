#pragma once

#include <mongocxx/client.hpp>
#include <memory>
#include <string>
#include <vector>

#include "models/Album.h"
#include "models/Image.h"

class AlbumService {
public:
    static bool initialize(const std::string& connectionString);

    static std::string createAlbum(const Album& album);
    static Album getAlbumById(const std::string& id);
    static Album getAlbumByToken(const std::string& token);
    static std::vector<Album> getAllAlbums(int page = 1, int limit = 10);
    static std::vector<Album> getAlbumsByStatus(AlbumStatus status, int page = 1, int limit = 10);
    static bool deleteAlbum(const std::string& id);

    static bool submitAlbum(const std::string& id);
    static bool publishAlbum(const std::string& id);
    static bool archiveAlbum(const std::string& id);

    static std::string generateAdminToken();
    static bool checkTokenValidity(const std::string& token);
    static bool validateAndConsumeToken(const std::string& token, const std::string& albumId);

    static bool addImageToAlbum(const std::string& albumId, const Image& image);
    static bool updateImageStatus(const std::string& albumId,
                                  const std::string& imageId,
                                  ImageStatus status,
                                  const std::string& nsfwReason = "");

private:
    static std::shared_ptr<mongocxx::client> client;
    static std::string dbName;
    static std::string albumsCollection;

    static std::string nowEpochString();
};
