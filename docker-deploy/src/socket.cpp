#include "socket.hpp"
int Socket::BuildSocket(){
    int proxy_fd, new_fd;
    int rv;
    int yes=1;
    struct addrinfo hints, *servinfo;
    char hostname[1024];
    memset(hostname, 0, sizeof(hostname));
    if (gethostname(hostname, sizeof(hostname)) == -1) {
        std::cerr << "Cannot get hostname" << std::endl;
        exit(EXIT_FAILURE);
    }
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    if ((rv = getaddrinfo(hostname, PORT, &hints, &servinfo)) != 0) {
        std::cerr << "Cannot get address information" << std::endl;
        exit(EXIT_FAILURE);
    }
    proxy_fd = socket(servinfo->ai_family,
                          servinfo->ai_socktype,
                          servinfo->ai_protocol);
    if (proxy_fd == -1) {
        std::cerr << "Cannot create socket" << std::endl;
        exit(EXIT_FAILURE);
    }
    rv = setsockopt(proxy_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (rv == -1) {
        std::cerr << "setsockopt" << std::endl;
        exit(EXIT_FAILURE);
    }
    rv = bind(proxy_fd, servinfo->ai_addr, servinfo->ai_addrlen);
    if (rv == -1) {
        close(proxy_fd);
        std::cerr << "Cannot bind" << std::endl;
        exit(EXIT_FAILURE);
    }
    rv= listen(proxy_fd, 120);
    if (rv == -1) {
        std::cerr << "Cannot listen on socket" << std::endl;
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(servinfo);
    return proxy_fd;
}

int Socket::ConnectTo(Request *target){
    char* host=new char[target->host.length()+1];
    strcpy (host, target->host.c_str());
    char* port=new char[target->port.length()+1];
    strcpy (port, target->port.c_str());
    int socketfd;
    struct addrinfo hints, *servinfo;
    int rv;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if ((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0) {
        std::cerr << "Cannot get address information" <<std::endl;
        return EXIT_FAILURE;
    }
    
    socketfd = socket(servinfo->ai_family,
                      servinfo->ai_socktype,
                      servinfo->ai_protocol);
    if (socketfd == -1) {
        std::cerr << "Cannot create socket" << std::endl;
        return EXIT_FAILURE;
    }
    rv = connect(socketfd, servinfo->ai_addr, servinfo->ai_addrlen);
    if(rv == -1){
        std::cerr<<"Cannot connect"<<std::endl;
        return EXIT_FAILURE;
    }
    freeaddrinfo(servinfo);
    delete[] host;
    delete[] port;
    return socketfd;
}
