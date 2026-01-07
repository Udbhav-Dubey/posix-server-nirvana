#pragma once
#include <string>
class HttpResponse;
class SongLibrary;
class Router{
    private:
        SongLibrary &library;
    public:
        HttpResponse::route(const std::string &path);
};
