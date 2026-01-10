#include "HttpResponse.h"
#include "Router.h"
#include "Songs.h"
#include "utils.h"
#include <iostream>
Router::Router(SongLibrary &lib):library(lib){};
HttpResponse Router::route(const std::string &path){
    if (path=="/api/songs"){
        return HttpResponse::json(library.ToJson());
    }
    if (path.rfind("/api/play/",0)==0&&path.length()>10){
        std::string id_string=path.substr(10);
        try{
            int id=std::stoi(id_string);
            const Song*song=library.getSong(id);
            if (song){
                std::string data;
                if (FileReader::read(song->getmp3_file(),data,true)){
                    HttpResponse resp(200,"audio/mpeg");
                    resp.SetBody(data);
                    return resp;
                }
            }
        }
        catch(const std::exception &e){
            return HttpResponse::notfound();
        }
    }
    if (path.rfind("/play/",0)==0&&path.length()>6){
        std::string id_string=path.substr(6);
        try{
            int id=std::stoi(id_string);
            const Song*song=library.getSong(id);
            if (song){
                std::string data;
                if (FileReader::read(song->getmp3_file(),data,true)){
                    HttpResponse resp(200,"audio/mpeg");
                    resp.SetBody(data);
                    return resp;
                }
            }
        }
        catch(const std::exception &e){
            return HttpResponse::notfound();
        }
    }
    if (path.rfind("/lyrics/",0)==0&&path.length()>8){
        std::string id_string=path.substr(8);
        try{
            int id=std::stoi(id_string);
            const Song*song=library.getSong(id);
            if (song&&!song->getlyrics_file().empty()){
                std::string data;
                if (FileReader::read(song->getlyrics_file(),data,false)){
                    HttpResponse resp(200,"text/plain");
                    resp.SetBody(data);
                    return resp;
                }
            }
        }
        catch(const std::exception &e){
            return HttpResponse::notfound();
        }
    }
    if (path.rfind("/download/",0)==0&&path.length()>10){
        std::string id_string=path.substr(10);
        try{
            int id=std::stoi(id_string);
            const Song*song=library.getSong(id);
            if (song){
                std::string data;
                if (FileReader::read(song->getmp3_file(),data,true)){
                    HttpResponse resp(200,"audio/mpeg");
                    resp.setContentDisposition(song->getmp3_file());
                    resp.SetBody(data);
                    return resp;
                }
            }
        }
        catch(const std::exception &e){
            return HttpResponse::notfound();
        }
    }
    if (path=="/"||path=="/index.html"){
        std::string html;
        std::vector<std::string> paths = {"../index.html", "index.html", "./index.html"};
        
        for (const auto& filepath : paths) {
            if (FileReader::read(filepath, html, false)) {
                std::cout << "Successfully loaded index.html from: " << filepath << "\n";
                HttpResponse resp(200,"text/html");
                resp.SetBody(html);
                return resp;
            }
        }
        
        std::cerr << "[ERROR] Could not find index.html in any of the attempted paths\n";
    }
    return HttpResponse::notfound();
}
