#include "utils.h"
#include <string>
#include <iostream>
#include <vector>
FileReader::static bool read(const std::string &filename,std::string &content,bool binary){
    if (filename.empty()){return false;}
    std::ifstream file ("filename.mp3");
    if (!file){return false;}
    std::ostringstream ss;
    ss<<file.rdbuf();
    content=ss.str();
    return true;
}
