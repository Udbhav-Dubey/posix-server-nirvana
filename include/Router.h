#pragma once
#include <string>
#include "HttpResponse.h"
#include "Songs.h"
class HttpResponse;
class SongLibrary;
class Router{
    private:
        SongLibrary &library;
    public:
        Router(SongLibrary &lib);
        HttpResponse route(const std::string &path);
};
