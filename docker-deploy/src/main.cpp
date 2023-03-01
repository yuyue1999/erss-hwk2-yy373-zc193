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
#include <pthread.h>
#include <time.h>
#include <fstream>
#include "Proxy.hpp"
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

class Helper{
public:
    int server_fd;
    int thread_id;
    cache * c;
    Log * LOG;
    Helper(int s_fd,int thread_id,cache * target,Log * targetLOG):
    server_fd(s_fd),thread_id(thread_id),c(target),LOG(targetLOG){}
};



std::string getIp(Request* Parsed){
    int status;
    struct addrinfo hints, *res;
    char ipstr[INET_ADDRSTRLEN];
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    char* host=new char[Parsed->host.length()+1];
    strcpy (host, Parsed->host.c_str());
    if ((status = getaddrinfo(host, NULL, &hints, &res)) != 0) {
        std::cerr << "Cannot get address information" <<std::endl;
        return NULL;
    }
    void *addr;
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
    addr = &(ipv4->sin_addr);
    inet_ntop(res->ai_family, addr, ipstr, sizeof ipstr);
    std::string ip(ipstr);
    delete[] host;
    return ip;
}

void *handle(void* para){
    Helper * p = (Helper *) para;
    int thread_id=p->thread_id;
    int browser_fd=p->server_fd;
    char temp[131072] = {0};
    int size = recv(browser_fd, temp, sizeof(temp), 0);
    if(size==0){
      return NULL;
    }
    std::string request(temp);
    Request * Parsed_R = new Request(request);
    std::string method = Parsed_R->method;
    pthread_mutex_lock(&mutex);
    std::string ip=getIp(Parsed_R);
    time_t nowTime =time(0);
    //const char * data = ctime(&now);
    struct tm * gm =gmtime(&nowTime);
    std::string tm = asctime(gm);
    *(p->LOG) << thread_id << ": \"" << Parsed_R->request_line << "\" from "<< ip << " @ " << tm;
    pthread_mutex_unlock(&mutex);
    Socket s;
    int Origianl_Server=s.ConnectTo(Parsed_R);
    Proxy *P=new Proxy(browser_fd,Origianl_Server);
    if(method=="CONNECT"){
        pthread_mutex_lock(&mutex);
    *(p->LOG) << thread_id << ": Responding \"HTTP/1.1 200 OK\""<<"\n";
    pthread_mutex_unlock(&mutex);
    
        P->ConnectRequest();
	
        pthread_mutex_lock(&mutex);
        *(p->LOG)<< thread_id << ": Tunnel closed"<<"\n";
        pthread_mutex_unlock(&mutex);
        delete Parsed_R;
    }else if(method=="GET"){
      //std::cout<<"Put1"<<std::endl;
        Response* Getcache;
        bool allowCache=true;
        if(Parsed_R->CacheControl.find("no-store")!=std::string::npos){
            allowCache=false;
        }
        pthread_mutex_lock(&mutex);
        Getcache = p->c->Get(Parsed_R, Origianl_Server,thread_id);
        pthread_mutex_unlock(&mutex);
        if(Getcache==NULL){
            allowCache=false;
        }
        if(allowCache==false){
            send(Origianl_Server, request.c_str(), request.size(), MSG_NOSIGNAL);

            pthread_mutex_lock(&mutex);
            *(p->LOG) << thread_id << ": Requesting \"" << Parsed_R->request_line << "\" from "<< Parsed_R->uri <<"\n";
            pthread_mutex_unlock(&mutex);

            Response * newRes=P->GetRequest();
            if(newRes!=NULL){
                pthread_mutex_lock(&mutex);
                *(p->LOG) << thread_id << ": Received \"" << newRes->firstLine<< " \" from " << Parsed_R->uri<<"\n";
                pthread_mutex_unlock(&mutex);

                pthread_mutex_lock(&mutex);
                *(p->LOG)<<thread_id<<": Responding \"" <<newRes->firstLine << "\""<<"\n";
                pthread_mutex_unlock(&mutex);
                
                pthread_mutex_lock(&mutex);
                p->c->Put(newRes, Parsed_R,thread_id);
                pthread_mutex_unlock(&mutex);
            }else {
                pthread_mutex_lock(&mutex);
                *(p->LOG)<<thread_id<<": Responding \"" <<"HTTP/1.1 502 Bad Gateway" << "\""<<"\n";
                pthread_mutex_unlock(&mutex);
              }
        }else{
            send(browser_fd,Getcache->response.c_str(),Getcache->response.size(), MSG_NOSIGNAL);
            pthread_mutex_lock(&mutex);
            *(p->LOG)<<thread_id<<": in cache, valid"<<"\n";
            pthread_mutex_unlock(&mutex);
        }
    }else if (method=="POST"){
        send(Origianl_Server, request.c_str(), request.size(), MSG_NOSIGNAL);
        pthread_mutex_lock(&mutex);
        *(p->LOG) << thread_id << ": Requesting \"" << Parsed_R->request_line << "\" from "<< Parsed_R->uri <<"\n";
        pthread_mutex_unlock(&mutex);
        Response * newRes=P->PostRequest();
        pthread_mutex_lock(&mutex);
        *(p->LOG) << thread_id << ": Received \"" << newRes->firstLine<< " \" from " << Parsed_R->uri<<"\n";
        pthread_mutex_unlock(&mutex);
        pthread_mutex_lock(&mutex);
        *(p->LOG)<<thread_id<<": Responding \"" <<newRes->firstLine << "\""<<"\n";
        pthread_mutex_unlock(&mutex);
        delete Parsed_R;
        delete newRes;
    }else {
        std::string error_code = "HTTP/1.1 400 Bad Request";
        send(browser_fd,error_code.c_str(),error_code.size(),MSG_NOSIGNAL);
        pthread_mutex_lock(&mutex);
        *(p->LOG)<<thread_id<<": Responding \"" <<error_code << "\""<<"\n";
        pthread_mutex_unlock(&mutex);
        delete Parsed_R;
    }
    delete p;
    delete P;
    close(Origianl_Server);
    close(browser_fd);
    return NULL;
}

int main(int argc, const char * argv[]) {
  //Log LOG("/home/yy373/ECE568HW2/ECE568HW2/proxy.log", std::ofstream::out);
  Log LOG("/var/log/erss/proxy.log", std::ofstream::out);
  Socket s;
    int proxy_fd=s.BuildSocket();
    int thread_id = 0;
    cache c(&LOG);
    /*if(daemon(0,0) == -1){
        std::cout<<"error\n"<<std::endl;
        exit(-1);
    }*/
    while(true){
        struct sockaddr_storage connector_addr;
        socklen_t addr_len = sizeof(connector_addr);
        int browser_fd= accept(proxy_fd, (struct sockaddr *)&connector_addr, &addr_len);
        if (browser_fd == -1) {
            std::cerr << "Cannot accept connection on that socket" << std::endl;
            continue;
        }
        pthread_mutex_lock(&mutex);
        thread_id++;
        Helper * parameter = new Helper(browser_fd, thread_id,&c,&LOG);
        pthread_mutex_unlock(&mutex);
        pthread_t thread;
        pthread_create(&thread, NULL, handle, parameter);
    }
    return 0;
}
