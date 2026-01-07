#pragma once
#include <mutex>
class Router;
class SongLibrary;
class TcpServer{
    private:
        int port;
        int server_socket;
        SongLibrary library;
        Router router;
        std::mutex cout_mutex;
        void handleSock(int client_sock);
    public:
        bool start();
        void run();
};
