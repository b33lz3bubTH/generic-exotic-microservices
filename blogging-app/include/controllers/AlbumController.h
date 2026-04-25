#pragma once

#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>

class AlbumController {
public:
    explicit AlbumController(Pistache::Address address);

    void init(size_t threads = 2);
    void start();
    void shutdown();

private:
    void setupRoutes();

    void getPublishedAlbums(const Pistache::Rest::Request& req, Pistache::Http::ResponseWriter response);
    void getAlbumById(const Pistache::Rest::Request& req, Pistache::Http::ResponseWriter response);
    void getAlbumStats(const Pistache::Rest::Request& req, Pistache::Http::ResponseWriter response);

    void createAlbum(const Pistache::Rest::Request& req, Pistache::Http::ResponseWriter response);
    void uploadImagesToAlbum(const Pistache::Rest::Request& req, Pistache::Http::ResponseWriter response);
    void submitAlbumDraft(const Pistache::Rest::Request& req, Pistache::Http::ResponseWriter response);
    void getAllAlbums(const Pistache::Rest::Request& req, Pistache::Http::ResponseWriter response);
    void getAlbumDetailsAdmin(const Pistache::Rest::Request& req, Pistache::Http::ResponseWriter response);
    void approveImage(const Pistache::Rest::Request& req, Pistache::Http::ResponseWriter response);
    void rejectImage(const Pistache::Rest::Request& req, Pistache::Http::ResponseWriter response);
    void flagImageNSFW(const Pistache::Rest::Request& req, Pistache::Http::ResponseWriter response);
    void publishAlbum(const Pistache::Rest::Request& req, Pistache::Http::ResponseWriter response);
    void archiveAlbum(const Pistache::Rest::Request& req, Pistache::Http::ResponseWriter response);
    void deleteAlbum(const Pistache::Rest::Request& req, Pistache::Http::ResponseWriter response);

    bool verifyAdminKey(const Pistache::Rest::Request& req) const;
    bool rateLimitUpload(const std::string& key);

    std::shared_ptr<Pistache::Http::Endpoint> httpEndpoint;
    Pistache::Rest::Router router;
};
