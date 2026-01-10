#include "Songs.h"
#include "utils.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
namespace fs=std::filesystem;
Song::Song(int id,const std::string&name,const std::string &mp3,const std::string &lyrics="")
    :id(id),name(name),mp3_file(mp3),lyrics_file{lyrics}{}
int Song::getId() const {
    return id;
}
std::string Song::getName() const {
    return name;
}
std::string Song::getmp3_file() const {
    return mp3_file;
}
std::string Song::getlyrics_file() const {
    return lyrics_file;
}
std::string Song::tojson() const{
    std::ostringstream ss;
        ss<<"   {\n";
        ss<<"    \"id\": "<<id<<",\n";
        ss<<"    \"name\": \""<<name<<"\",\n";
        ss<<"    \"mp3\":\"" <<mp3_file<<"\",\n";
        ss<<"    \"lyrics\": \""<<lyrics_file<<"\"\n";
        ss<<"   }";
        return ss.str();
}
void SongLibrary::scanDirectory(const std::string&directory="data"){
    songs.clear();
    std::vector<std::string>mp3Files;
    try{
        for (const auto &entry:fs::directory_iterator(directory)){
            if (entry.is_regular_file()){
                std::string filename=entry.path().filename().string();
                if (filename.length()>3&&filename.substr(filename.length()-4)==".mp3"){
                    mp3Files.push_back(filename);
                }
            }
        }
        std::sort(mp3Files.begin(),mp3Files.end());
        for (size_t i=0;i<mp3Files.size();i++){
            std::string base=mp3Files[i].substr(0,mp3Files[i].length()-4);
            std::string lyricsfile=directory+"/"+base+".txt";
            if (!fs::exists(lyricsfile)){
                lyricsfile="";
            }
            songs.emplace_back(i,base,directory+"/"+mp3Files[i],lyricsfile);
        }
        std::cout << "discovered " << songs.size() << " songs\n";
        for (const auto &s:songs){
            std::cout << " ["<<s.getId()<<"] "<<s.getName();
            if (!s.getlyrics_file().empty()){
                std::cout << "lyrics found ";
            }
            std::cout << "\n";
        }
    }
    catch(const fs::filesystem_error&e){
        std::cerr<<"[error] in filesystem "<< e.what()<<"\n";
    }
}
const Song* SongLibrary::getSong(int id)const {
    for (const auto &s:songs){
        if (s.getId()==id){
            return &s;
        }
    }
    return NULL;
}
size_t SongLibrary::count() const{
    return songs.size();
}
std::string SongLibrary::ToJson() const{
    std::ostringstream json;
    json<<"{\n";
    json<<" \"count\": " << songs.size()<<",\n";
    json<<" \"songs\":[\n";
    for (size_t i=0;i<songs.size();i++){
        json<<songs[i].tojson();
        if (i<songs.size()-1)json<<",";
        json<<"\n";
     }
    json<<"]\n";
    json<<"}\n";
    return json.str();
}
