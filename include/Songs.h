#pragma once
#include <iostream>
#include <string>
class Song{
    private:
        int id;
        std::string mp3_file;
        std::string lyrics_file;
        std::string name;
    public:
        int getId() const ;
        std::string getName() const ;
        std::string getmp3_file()const ;
        std::string getlyrics_file() const;
        std::string tojson()const;
};
class SongLibrary{
    private:
        std::vector<Song>songs;
    public:
        void scanDirectory(const std::string&directory);
        const Song*getSong(int id)const;
        size_t count()const;
        std::string ToJson()const;
};

