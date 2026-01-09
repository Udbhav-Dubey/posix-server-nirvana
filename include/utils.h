#pragma once
#include <fstream>
#include <string>
class FileReader{
    public:
        static bool read(const std::string&filename,std::string&content,bool binary);
};
