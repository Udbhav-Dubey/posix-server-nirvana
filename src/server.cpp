#include "server.h"
#include "HttpResponse.h"
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <netdb.h>
#include <chrono>
#include <cstring>
#include <poll.h>
#include <fcntl.h>
#include <unordered_map>
#include <string>
struct client{
    int id;
    std::string write_buf{};
    //std::string read_buf{};
};
void remove_client(int fd,std::unordered_map<int,client>&mp,std::vector<pollfd>&poll_fds){
    auto it=mp.find(fd);
    if (it!=mp.end()){
        std::cout <<it->first << "[-] Client left the room \n";
        mp.erase(it);
    }
    close(fd);
    poll_fds.erase(
        std::remove_if(poll_fds.begin(),poll_fds.end(),[fd](const pollfd&pfd){return pfd.fd==fd;}),
        poll_fds.end());
}
bool set_nonblocking(int fd){
    int flags=fcntl(fd,F_GETFL,0);
    if (flags==-1){
        perror("fnctl f_getfl");
        return false;
    }
    if (fcntl(fd,F_SETFL,flags|O_NONBLOCK)==-1){
        perror("fcntl f_setfl or nonblock");
        return false;
    }
    return true;
}
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
    if (server_socket<0){
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
        return false;
    }
    if (listen(server_socket,10)<0){
        perror("listen");
        close(server_socket);
        return false;
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
    poll_fds.push_back({.fd=server_socket,.events=POLLIN,.revents=0});
    std::unordered_map<int,client> mp;
    while(true){
        int red=poll(poll_fds.data(),poll_fds.size(),-1);
        if (red<0){
            perror("poll");
            break;
        }
        for (size_t i=0;i<poll_fds.size();i++){
            pollfd &pfd=poll_fds[i];
            if  (pfd.revents&(POLLHUP|POLLERR|POLLNVAL)){
                remove_client(pfd.fd,mp,poll_fds);
                i--;
                continue;
            }
            if ((pfd.fd==server_socket)&&(pfd.revents&POLLIN)){
               sockaddr_in client_addr{};
               socklen_t client_len=sizeof(client_addr);
               int client_sock=accept(server_socket,(sockaddr*)&client_addr,&client_len);
               if (client_sock<0){
                continue;
               }
                if(!set_nonblocking(client_sock)){
                    close(client_sock);
                    continue;
                }
                client cl;
                cl.id=client_sock;
                poll_fds.push_back({.fd=client_sock,.events=POLLIN,.revents=0});
                std::cout << "New client : fd = " << client_sock << "\n";
                mp[client_sock]=cl;             
            }
            else if ((pfd.fd!=server_socket)&&(pfd.revents&POLLIN)){
                //auto StartTime=std::chrono::steady_clock::now();
                char buffer[4096];
                ssize_t n=recv(pfd.fd,buffer,sizeof(buffer),0);
                if (n<0){
                    if (errno==EAGAIN||errno==EWOULDBLOCK){
                        continue;
                    }
                    perror("recv");
                    remove_client(pfd.fd,mp,poll_fds);
                    i--;
                    continue;
                }
                if (n==0){
                    std::cout << pfd.fd << " left the room\n";
                    remove_client(pfd.fd,mp,poll_fds);
                    i--;
                    continue;
                }
                buffer[n]='\0';
                try{
                    std::string request(buffer);
                    if (request.find("HTTP/")==std::string::npos){
                        remove_client(pfd.fd,mp,poll_fds);
                        i--;
                        continue;
                    }
                    std::string path="/";
                    size_t pos=request.find(" ");
                    if (pos!=std::string::npos){
                        size_t pos2=request.find(" ",pos+1);
                        if (pos2!=std::string::npos){
                            path=request.substr(pos+1,pos2-pos-1);
                        }
                    }
                    std::cout << " Request  " << path ;
                    HttpResponse response=router.route(path);
                    std::string response_data=response.build();
                  //  mp[pfd.fd].read_buf.append(response_data,n);
                   // mp[pfd.fd].write_buf+=mp[pfd.fd].read_buf;
                    //mp[pfd.fd].read_buf.clear();
                    mp[pfd.fd].write_buf+=response_data;
                    pfd.events|=POLLOUT;
                }
                catch(const std::exception&e){
                    std::cerr<<"[error] in client handling : " << e.what() << "\n";
                }
            }
            if ((pfd.fd!=server_socket)&&(pfd.revents&POLLOUT)){
                if (!mp[pfd.fd].write_buf.empty()){
                    int sendy=send(pfd.fd,mp[pfd.fd].write_buf.data(),mp[pfd.fd].write_buf.size(),0);
                    if (sendy<=0){
                    if (errno==EAGAIN||errno==EWOULDBLOCK){
                        continue;
                    }
                        perror("send");
                        remove_client(pfd.fd,mp,poll_fds);
                        i--;
                        continue;
                    }
                    
                    mp[pfd.fd].write_buf.erase(0,sendy);
                    if (mp[pfd.fd].write_buf.empty()){
                        pfd.events&=~POLLOUT;
                        remove_client(pfd.fd,mp,poll_fds);
                        i--;
                    }
                }
            }
        }
    }
    for (auto &pair:mp){
        close(pair.first);
    }
    close(server_socket);
    return ;
}
 
