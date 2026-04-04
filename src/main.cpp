#include "server.h"
#include "store.h"
#include "api.h"

#include <csignal>
#include <iostream>
#include <stdexcept>

static HttpServer* g_server = nullptr;

static void handle_signal(int) {
    std::cout << "\nShutting down...\n";
    if (g_server) g_server->stop();
}

int main(int argc, char* argv[]) {
    int port = 8080;
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (...) {
            std::cerr << "Usage: " << argv[0] << " [port]\n";
            return 1;
        }
    }

    signal(SIGINT,  handle_signal);
    signal(SIGTERM, handle_signal);

    ListStore store;
    Router    router;
    register_api_routes(router, store);

    HttpServer server("0.0.0.0", port);
    g_server = &server;
    server.use(router);

    try {
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
