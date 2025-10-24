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
            std::cout << " Lyrics available ";
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
    void displayAll() const  {
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
class CliPlayer{
private:
    SongLibrary library;
    void clearScreen(){
        #ifdef _WIN32
            system("cls");
        #else 
            system("clear");
        #endif
    }
    void displayHeader(){
        std::cout << "Kakarotshyper's player: \n\n ";
    }
    void displaySongs(){
        std::cout << "Available Songs:\n";
        library.displayAll();
        std::cout <<"\n";
    }
    void displayCommands(){
        std::cout << "Commands :\n";
        std::cout << "<id> -Play song by entering the id \n";
        std::cout << "[space] -Pause/Resume\n";
        std::cout << "q -Stop the current song\n";
        std::cout << "l -List all songs \n";
        std::cout << ":wq - to exit the player\n";
        std::cout << "\n";
    }
        // more functions to write here now after music player is written that is core thing
        // 2 important functions that are play command and handle inputs that will be done here in private only;
public:
    bool initialize(){
        clearScreen();
        std::cout << "scanning for mp3 files : \n\n\n";
        library.scanDirectory(".");
        if (library.count()==0){
            std::cout << "\n no mp3 found in current directory .\n";
            std::cout << "please add some .mp3 to get this thing running \n";
            return false;
        }
        std::cout << "Library loaded successfully \n";
        //std::this_thread::sleep_for(std::chrono::seconds(1));
        return true;
    }
    void run(){
        while(true){
            clearScreen();
            displayHeader();
           // displayStatus();
            displaySongs();
            displayCommands();
            std::cout << "-> ";
            std::string input;
            std::getline(std::cin,input);
           // processCommand(input);
        }
    }
};
int main (){
    CliPlayer player;
    if (!player.initialize()){
        return 1;
    }
    player.run();
   // SongLibrary library;
   // library.scanDirectory(".");
  //  library.displayAll();
    return 0;
}
