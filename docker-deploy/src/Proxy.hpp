#ifndef Proxy_hpp
#define Proxy_hpp

#include <stdio.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>
#include <iostream>
#include "socket.hpp"
#endif /* Proxy_hpp */

class Proxy{
public:
    int browser_fd;
    int originalServer_fd;
    Proxy(int b_fd,int OS_fd):browser_fd(b_fd),originalServer_fd(OS_fd){
    }
    void ConnectRequest();
    Response* GetRequest();
    Response* PostRequest();
    bool isContentEnding(std::vector<char> &store,int Clength);
};
