#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstdlib>
namespace fs=std::filesystem;
class Song{
private:
    int id;
    std::string mp3_file;
    std::string lyrics_file;
    std::string name;
public:
    Song(int id,const std::string &name,const std::string &mp3,const std::string &lyrics=" ")
        :id(id),name(name),mp3_file(mp3),lyrics_file(lyrics){}
    int getId() const{return id;}
    std::string getName()const {return name;}
    std::string getlyrics_file() const {return lyrics_file;}
    bool hasLyrics() const {return !lyrics_file.empty();}
    void display() const {
        std::cout << " " << id << " " << name;
        if (hasLyrics()){
            std::cout << "Lyrics available ";
        }
        std::cout << "\n";
    }
};
class SongLibrary{
private:
    std::vector<Song>songs;
    std::unordered_map<int,size_t> songMap;
public:
    void scanDirectory(const std::string &directory="."){
        songs.clear();
        songMap.clear();
        std::vector<std::string>Mp3Files;
        try{
            for (const auto&entry:fs::directory_iterator(directory)){
                if (entry.is_regular_file()){
                    std::string filename=entry.path().filename().string();
                    if (filename.size()>=4&&filename.substr(filename.length()-4)==".mp3"){
                        Mp3Files.push_back(filename);
                    }
                }
            }
            std::sort(Mp3Files.begin(),Mp3Files.end());
            for (size_t i=0;i<Mp3Files.size();i++){
                std::string base=Mp3Files[i].substr(0,Mp3Files[i].size()-4);
                std::string lyricsfile=base+".txt";
                if (!fs::exists(lyricsfile))lyricsfile="";
                songs.emplace_back(i,base,Mp3Files[i],lyricsfile);
                songMap[i]=songs.size()-1;
            }
            std::cout << "Discovered "<< songs.size()<<" songs\n";
        }
        catch(const fs::filesystem_error &e){
            std::cout << "error in filesystem " << e.what() << "\n";
        }
    }
    const Song* getSong(int id)const {
        auto it=songMap.find(id);
        if (it!=songMap.end()){
            return &songs[it->second];
        }
        return nullptr;
    }
    size_t count () const {
        return songs.size();
    }
    void displayAll() const {
        for (const auto &song:songs){
            song.display();
        }
    }
};
class FileReader{
    public:
        static bool read(const std::string &filename,std::string &content){
            if (filename.empty()) return false;
            std::ifstream file(filename,std::ios::in);
            if (!file.is_open())return false;
            std::ostringstream ss;
            ss<<file.rdbuf();
            content=ss.str();
            return true;
        }
};
class CliPlayer{};
int main (){
//    CliPlayer player;
  //  player.run();
    return 0;
}
