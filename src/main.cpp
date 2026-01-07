#include <iostream>
#include <signal.h>
#include <server.h>
int main (){
    signal(SIGPIPE,SIG_IGN);
    TcpServer server(28333);
    if (!server.start()){
        return 1;
    }
    server.run();
    return 0;
}
