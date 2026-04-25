#include <pistache/net.h>

#include <iostream>

#include "controllers/AlbumController.h"
#include "services/AlbumService.h"
#include "services/ImageService.h"

int main() {
    if (!AlbumService::initialize("")) {
        std::cerr << "Failed to initialize album service.\n";
        return 1;
    }

    if (!ImageService::initialize()) {
        std::cerr << "Failed to initialize local image storage.\n";
        return 1;
    }

    Pistache::Address addr(Pistache::Ipv4::any(), Pistache::Port(8080));
    AlbumController server(addr);
    server.init(4);

    std::cout << "Photo sharing service (Pistache) running on :8080" << std::endl;
    server.start();

    return 0;
}
