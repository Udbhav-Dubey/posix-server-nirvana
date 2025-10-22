#include <iostream>
#include <vector>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
namespace fs=std::filesystem;

class Song{
    private:
        int id;
        std::string mp3_file;
        std::string lyrics_file;
        std::string name;
    public :
        Song(int id,const std::string &name,const std::string &mp3,const std::string &lyrics="")
            : id(id),name(name),mp3_file(mp3),lyrics_file(lyrics){}
        int getId() const {return id;}
        std::string getName() const{return name;}
        std::string getmp3_file()const {return mp3_file;}
        std::string getlyrics_file() const {return lyrics_file;}
        
        std::string tojson()const{
            std::ostringstream ss;
            ss<<"   {\n";
            ss<<"    \"id\": "<<id<<",\n";
            ss<<"    \"name\": \""<<name<<"\",\n";
            ss<<"    \"mp3\":\"" <<mp3_file<<"\",\n";
            ss<<"    \"lyrics\": \""<<lyrics_file<<"\"\n";
            ss<<"   }";
            return ss.str();
        }
};
class SongLibrary{
    private:
        std::vector<Song>songs;
    public :
        void scanDirectory(const std::string&directory="."){
            songs.clear();
            std::vector<std::string>mp3Files;
            try{
                for (const auto &entry :fs::directory_iterator(directory)){
                    if (entry.is_regular_file()){
                        std::string filename=entry.path().filename().string();
                        if (filename.length()>=4&&filename.substr(filename.length()-4)==".mp3"){
                            mp3Files.push_back(filename);
                        }
                    }
                }
                std::sort(mp3Files.begin(),mp3Files.end());
                for (size_t i=0;i<mp3Files.size();i++){
                    std::string base=mp3Files[i].substr(0,mp3Files[i].size()-4);
                    std::string lyricsfile=base+".txt";
                    if (!fs::exists(lyricsfile)){
                        lyricsfile=""; 
                    }
                    songs.emplace_back(i,base,mp3Files[i],lyricsfile);
                }
                std::cout << "discovered " << songs.size() << " songs\n";
                for (const auto&song:songs){
                    std::cout << "  ["<<song.getId() << "] "<<song.getName();
                    if (!song.getlyrics_file().empty()){
                        std::cout << "lyrics found ";
                    }
                    std::cout << "\n";
                }

            }
            catch (const fs::filesystem_error &e){
                std::cerr<<"[error] filesystem error : " <<e.what() << "\n";
            }
        }
        const Song*getSong(int id)const{
            for (const auto &song:songs){
                if (song.getId()==id){
                    return &song;
                }
            }
            return nullptr;
        }
        size_t count() const{
            return songs.size();
        }
        std::string ToJson() const{
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
};
class FileReader{
    public :
        static bool read(const std::string&filename,std::string&content,bool binary=false){
            if (filename.empty())return false;
            
            std::ifstream file(filename,binary?std::ios::binary:std::ios::in);
            if (!file.is_open())return false;
            std::ostringstream ss;
            ss<<file.rdbuf();
            content=ss.str();
            return true;
        }
};

class HttpResponse{
    private:
        int status_code;
        std::string content_type;
        std::string body_;
    public:
        HttpResponse(int status=200,const std::string&content_type="text/plain")
            :status_code(status),content_type(content_type){}
        void SetBody(const std::string &body){body_=body;}
        std::string build() const{
            std::ostringstream response ;
            std::string status_text=(status_code==200)?"OK":"Not found";
            response << "HTTP/1.1 " << status_code << " " << status_text << "\r\n";
            response << "Content-Type: "<<content_type<<"\r\n";
            response << "Content-Length: "<<body_.size()<<"\r\n";
            response << "Connection : close\r\n\r\n";
            response << body_;
            return response.str();
        }
        static HttpResponse notfound(){
            HttpResponse resp(404,"text/html");
            resp.SetBody("<html><body><h1> NOt found </h1></body></html>");
            return resp;
        }
        static HttpResponse json(const std::string &data){
            HttpResponse resp(200,"application/json");
            resp.SetBody(data);
            return resp;
        }
};
class Router{
    private:
        SongLibrary &library;
    public:
        Router(SongLibrary &lib):library(lib){};
        /*Router(){
            songs.push_back(Song(1,"Smells like teen sprit ","smells.mp3","smells.txt"));
            songs.push_back(Song(2,"Dumb","dumb.mp3","dumb.txt"));
            songs.push_back(Song(3,"All Apologies","apologies.mp3","apologies.txt"));
        }*/
        HttpResponse route(const std::string & path){
            if (path=="/api/songs"){
                return HttpResponse::json(library.ToJson());
            }
            if (path.rfind("/api/play/",0)==0&&path.length()>=10){
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
            if (path.rfind("/play/",0)==0&&path.length()>=7){
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
            if (path.rfind("/lyrics/",0)==0&&path.length()>=9){
                std::string id_string=path.substr(8);
                try{
                    int id=std::stoi(id_string);
                    const Song*song=library.getSong(id);
                    if (song && !song->getlyrics_file().empty()){
                        std::string data;
                        if (FileReader::read(song->getlyrics_file(),data)){
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
            if (path=="/"||path=="/index.html"){
                std::string html;
                if (FileReader::read("index.html",html)){
                    HttpResponse resp(200,"text/html");
                    resp.SetBody(html);
                    return resp;
                }
            }

            return HttpResponse::notfound();
        }
};
class TcpServer{
    private:
        int port;
        int server_socket;
        Router router;
        SongLibrary library;
    public :
        TcpServer(int port ): port(port),server_socket(-1),router(library){}
        ~TcpServer(){
            if (server_socket>=0){
                close(server_socket);
            }
        }
        bool start(){
            library.scanDirectory(".");
            if (library.count()<=0){
                std::cout << "no MP3 found \n";
                return false;
            }
            server_socket=socket(AF_INET,SOCK_STREAM,0);
            if (server_socket<0){
                perror("socket");
                return false;
            }
            int opt=1;
            setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
            sockaddr_in addr{};
            addr.sin_family=AF_INET;
            addr.sin_port=htons(port);
            addr.sin_addr.s_addr=INADDR_ANY;
        if (bind(server_socket,(sockaddr*)&addr,sizeof(addr))<0){
            perror("bind");
            return false;
        }
        if (listen(server_socket,10)<0){
            perror("listen");
            return false;
        }
        std::cout << "on port : " << port << "\n";
        std::cout << "http://localhost:"<<port<<"\n\n";
        return true;
    }
        void run(){
            while(true){
                sockaddr_in client_addr{};
                socklen_t client_len=sizeof(client_addr);
                int client_sock=accept(server_socket,(sockaddr*)&client_addr,&client_len);
                if (client_sock<0)continue;
                handleClient(client_sock);
            }
        }
    private:
        void handleClient(int client_sock){
            char buffer[4096];
            ssize_t n=recv(client_sock,buffer,sizeof(buffer)-1,0);
            if (n<=0){
                close(client_sock);
                return ;
            }
            buffer[n]='\0';

            std::string request(buffer);
            std::string path="/";
            size_t pos=request.find(" ");
            if (pos!=std::string::npos){
                size_t pos2=request.find(" ",pos+1);
                if (pos2!=std::string::npos){
                    path=request.substr(pos+1,pos2-pos-1);    
                }
            }
            std::cout << "Request  " << path << "\n";

            HttpResponse response = router.route(path);
            std::string response_data=response.build();

            size_t total=0;
            while(total<response_data.size()){
                ssize_t sent=send(client_sock,response_data.data()+total,response_data.size()-total,0);
                if (sent<=0)break;
                total+=sent;
            }
            close(client_sock);
        }
};
int main (){
    TcpServer server(28333);
    if (!server.start()){
        return 1;
    }
    server.run();
    return 0;
}

