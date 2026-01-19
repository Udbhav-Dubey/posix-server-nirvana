#pragma once
#include <mutex>
#include "Songs.h"
#include "Router.h"
#include <poll.h>
#include <vector>
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
        std::vector<pollfd> poll_fds;
    public:
        TcpServer(int port );
        bool start();
        void run();
        ~TcpServer();
};
