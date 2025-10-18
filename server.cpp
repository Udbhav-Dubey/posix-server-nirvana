#include <iostream>
#include <vector>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fstream>
#include <sstream>

#define port 28333
#define backlog 10

bool send_all(int sockfd,const char*data,size_t len){
    size_t total=0;
    while(total<len){
        ssize_t sent=send(sockfd,data+total,len-total,0);
        if (sent<=0)return false;
        total+=sent;
    }
    return true;
}
bool send_all(int sockfd,const std::string&data){
    return send_all(sockfd,data.data(),data.size());
}
struct Song {
    std::string mp3;
    std::string lyrics;
};
Song songs[4]={
    {},
    {"smells.mp3","smells.txt"},
    {"dumb.mp3","dumb.txt"},
    {"apologies.mp3","apologies.txt"}
};
bool read_file(const std::string &filename,std::string &out,bool binary=false){
    if (filename.empty())return false;
    std::ifstream file(filename,binary?std::ios::binary:std::ios::in);
    if (!file.is_open())return false;
    std::ostringstream ss;
    ss<<file.rdbuf();
    out=ss.str();
    return true;
}
void send_404(int client_sock) {
    std::string response = 
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "Content-Length: 47\r\n"
        "\r\n"
        "<html><body><h1>404 Not Found</h1></body></html>";
    send_all(client_sock, response);
}
void send_200(int client_sock, const std::string& contentType, const std::string& data) {
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Type: " << contentType << "\r\n";
    response << "Content-Length: " << data.size() << "\r\n";
    response << "Connection: close\r\n\r\n";
    
    send_all(client_sock, response.str());
    send_all(client_sock, data);
}
int main (){
    int sockfd=socket(AF_INET,SOCK_STREAM,0);
    if (sockfd<0){perror("socket");return 1;}
    int opt=1;
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    addr.sin_addr.s_addr=INADDR_ANY;
    
    if(bind(sockfd,(sockaddr*)&addr,sizeof(addr))<0){perror("bind");return 2;}
    if (listen(sockfd,backlog)<0){perror("listen");return 3;}
    std::cout << "Server now listining on port : " << port << "\n";
    while(true){
        sockaddr_in client_addr{};
        socklen_t client_len=sizeof(client_addr);
        int client_sock=accept(sockfd,(sockaddr*)&client_addr,&client_len);
        if (client_sock<0){perror("accept");continue;}

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET,&client_addr.sin_addr,ip,sizeof(ip));
        std::cout << "connection from " << ip << ": " << ntohs(client_addr.sin_port) << "\n";
        char buffer[4096];
        ssize_t n = recv(client_sock,buffer,sizeof(buffer)-1,0);
        if (n<=0){close(client_sock);continue;}
        buffer[n]='\0';
        std::string request(buffer);
        size_t pos=request.find(" ");
        std::string path="/";
        if (pos!=std::string::npos){
            size_t pos2=request.find(" ",pos+1);
            if (pos2!=std::string::npos){
                path=request.substr(pos+1,pos2-pos-1);
            }
        }
        std::string fileToSend;
        std::string contentType="text/plain";
        bool binary=false;
        if (path=="/" || path=="/index.html"){
            fileToSend="index.html";
            contentType="text/html";
        }
        else if (path.rfind("/play/",0)==0&&path.length()>=7){
            int id=path[6]-'0';
            if (id>=1&&id<=3){
                fileToSend=songs[id].mp3;
                contentType="audio/mpeg";
                binary=true;
            }
        }else if (path.rfind("/lyrics/",0)==0&&path.length()>=9){
            int id=path[8]-'0';
            if (id>=1&&id<=3){
                fileToSend=songs[id].lyrics;
                contentType="text/plain";
            }
        }
        std::string data;
        if (!read_file(fileToSend,data,binary)){
            std::string notFound="HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nConnection: close\r\nContent-Length: 47\r\n\r\n<html><body><h1>404 Not Found</h1></body></html>";
        send_all(client_sock,notFound);
        }
        else {
            std::ostringstream response;
            response << "HTTP/1.1 200 OK\r\n";
            response << "Content-Type: " << contentType<<"\r\n";
            response << "Content-Length: "<< data.size() << "\r\n";
            response << "Connection : close\r\n\r\n";
            send_all(client_sock,response.str());
            send_all(client_sock,data);
        }
        close(client_sock);
    }
    close(sockfd);
    return 0;
}
