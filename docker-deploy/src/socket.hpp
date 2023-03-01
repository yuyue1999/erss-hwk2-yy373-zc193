#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "cache.hpp"
#define PORT "12345"

#ifndef socket_hpp
#define socket_hpp

#include <stdio.h>

#endif /* socket_hpp */

class Socket{
public:
    int BuildSocket();
    int ConnectTo(Request * target);
};
