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
    std::cout << "http://localhost:"<<port<<"\n\n";
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
    std::cout <<"http:://"<<buffer<<":"<<port<<"\n";
    close(sock);
    return true;
}
void TcpServer::run(){
    while(true){
        sockaddr_in client_addr{};
        socklen_t client_len=sizeof(client_addr);
        int client_sock=accept(server_socket,(sockaddr *)&client_addr,&client_len);
        if (client_sock<=-1){continue;}
        std::thread(&TcpServer::handleClient,this,client_sock).detach();
    }
    }
void TcpServer::handleClient(int client_sock){
    auto StartTime=std::chrono::steady_clock::now();
    char buffer[4096];
    ssize_t n=recv(client_sock,buffer,sizeof(buffer),0);
    if (n<0){
        perror("recv");
        std::cout << "connection removed \n";
        close(client_sock);
        return ;
    }
    buffer[n]='\0';
    try{
        std::string request(buffer);
        std::string path="/";
        size_t pos=request.find(" ");
        if (pos!=std::string::npos){
            size_t pos2=request.find(" ",pos+1);
            if (pos2!=std::string::npos){
                path=request.substr(pos+1,pos2-pos-1);
            }
        }
        {
            std::lock_guard<std::mutex>lock(cout_mutex);
            std::cout << "[Thread " << std::this_thread::get_id()<<"] Request : " <<path;
        }
        std::cout << "Request   " << path  ;
        HttpResponse response=router.route(path);
        std::string response_data=response.build();
        size_t total=0;
        while(total<response_data.size()){
            ssize_t sent=send(client_sock,response_data.data()+total,response_data.size()-total,0);
            if (sent<=0)break;
            total+=sent;
        }
    }
    catch(const std::exception&e){
        std::cerr<<"[error] exceptions in handle clinet : " << e.what() << "\n";
    }
    close(client_sock);
    auto end_time=std::chrono::steady_clock::now();
    auto duration=end_time-StartTime;
    auto ms=std::chrono::duration<double,std::milli>(duration).count();
    {
        std::lock_guard<std::mutex>lock(cout_mutex);
        std::cout<<":: "<<std::fixed << std::setprecision(2)<<ms<<" ms\n";
    }
}

