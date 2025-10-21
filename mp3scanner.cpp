#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <sstream>
namespace fs=std::filesystem;

class Song {
    private:
        int id;
        std::string name;
        std::string mp3File;
        std::string lyricsfile;
    public :
        Song(int id,const std::string &name,const std::string &mp3File,const std::string&lyricsfile=" ")
            :id(id),name(name),mp3File(mp3File),lyricsfile(lyricsfile){}
        int getid() const {return id;}
        const std::string &getName()const {return name;}
        const std::string &getmp3File()const {return mp3File;}
        const std::string &getlyricsfile()const {return lyricsfile;}
        
        std::string toJson() const {
            std::ostringstream ss;
            ss<<"   {\n";
            ss<<"     \"id\": "<<id<<",\n";
            ss<<"     \"name\": \"" << name << "\",\n";
            ss<<"     \"mp3\":\"" <<mp3File << "\",\n";
            ss<<"     \"lyrics\": \"" << lyricsfile << "\"\n";
            ss<<"   }";
            return ss.str();
        }
};
class SongScanner{
    private:
        std::vector <Song> songs;
    public :
        void scanCurrentDirectory(){
            songs.clear();
        std::vector <std::string> mp3Files;
            try{
                for (const auto &entry:fs::directory_iterator(".")){
                    if (entry.is_regular_file()){
                        std::string filename=entry.path().filename().string();
                        if (filename.length()>4&&filename.substr(filename.length()-4)==".mp3"){
                            mp3Files.push_back(filename); 
                        }
                    }
                }
                std::sort(mp3Files.begin(),mp3Files.end());
            
            for (size_t i=0;i<mp3Files.size();i++){
                std::string base=mp3Files[i].substr(0,mp3Files[i].size()-4);
                std::string lyricsfile=base+".txt";
                if (!fs::exists(lyricsfile)){
                    lyricsfile=" ";
                }
                songs.emplace_back(i,base,mp3Files[i],lyricsfile);
            }
            std::cout << "auto discoverd  " << songs.size()<<" songs:\n";
            for (const auto&s:songs){
                std::cout << " [  " << s.getid() <<" ] " << s.getName();
                if (!s.getlyricsfile().empty()){
                    std::cout << " lyrics found ";
                    std::cout << "\n";
                }
            }
            }
            catch(const fs::filesystem_error&e){
                std::cerr<<"[Error] filesystem error :" << e.what()<<"\n";
            }
        }
        std::string toJson()const {
            std::ostringstream json;
            json << "{\n";
            json << "   \"count\": "<<songs.size()<<",\n";
            json << "   \"songs\":  [\n";
            for (size_t i=0;i<songs.size();++i){
                json << songs[i].toJson();
                if (i<songs.size()-1)json << " ,";
                json << "\n";
            }
            json << "]\n";
            json <<"}\n";
        return json.str();
        }
};
int main (){
    SongScanner scanner;
    scanner.scanCurrentDirectory();
    std::cout << scanner.toJson();
}

