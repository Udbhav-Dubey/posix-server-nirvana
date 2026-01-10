#pragma once
#include <mutex>
#include "Songs.h"
#include "Router.h"
class Router;
class SongLibrary;
class TcpServer{
    private:
        int port;
        int server_socket;
        SongLibrary library;
        Router router;
        std::mutex cout_mutex;
        void handleClient(int client_sock);
    public:
        TcpServer(int port );
        bool start();
        void run();
        ~TcpServer();
};
