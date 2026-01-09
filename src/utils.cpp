#include "utils.h"
#include <string>
#include <iostream>
#include <vector>
#include <sstream>

bool FileReader::read(const std::string &filename,std::string &content,bool binary) {
    if (filename.empty()) return false;
    std::ifstream file;
    if (binary)
        file.open(filename, std::ios::binary);
    else
        file.open(filename);
    if (!file) return false;
    std::ostringstream ss;
    ss << file.rdbuf();
    content = ss.str();
    return true;
}
