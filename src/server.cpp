#include "server.h"
#include "HttpResponse.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <netdb.h>
#include <chrono>
#include <cstring>
TcpServer::TcpServer(int port):port(port),server_socket(-1),router(library){}
TcpServer::~TcpServer(){
    if (server_socket>=0){
        close(server_socket);
    }
}
bool TcpServer::start(){
    library.scanDirectory("data");
    if (library.count()<=0){
        std::cout << "no mp3 files found \n";
        return false;
    }
    server_socket=socket(AF_INET,SOCK_STREAM,0);
    if (server_socket<=-1){
        perror("socket");
        return false;
    }
    int opt=1;
    setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    sockaddr_in serv,name,addr{};
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    addr.sin_addr.s_addr=INADDR_ANY;
    if (bind(server_socket,(sockaddr*)&addr,sizeof(addr))<0){
        perror("bind");
        close(server_socket);
        return 0;
    }
    if (listen(server_socket,10)<0){
        perror("listen");
        close(server_socket);
        return 0;
    }
    std::cout << "server listining on port : " << port << "\n";
    std::cout <<"localhost ---> " <<"http://localhost:"<<port<<"\n\n";
    socklen_t namelen=sizeof(name);
    char buffer[INET_ADDRSTRLEN];
    int sock=socket(AF_INET,SOCK_DGRAM,0);
    memset(&serv,0,sizeof(serv));
    serv.sin_family=AF_INET;
    serv.sin_port=htons(port);
    inet_pton(AF_INET,"8.8.8.8",&serv.sin_addr);
    connect(sock,(struct sockaddr *)&serv,sizeof(serv));
    getsockname(sock,(struct sockaddr *)&name,&namelen);
    inet_ntop(AF_INET,&name.sin_addr,buffer,sizeof(buffer));
    std::cout <<"ip+port ---> "<<"http://"<<buffer<<":"<<port<<"\n";
    close(sock);
    return true;
}
void TcpServer::run(){

}

