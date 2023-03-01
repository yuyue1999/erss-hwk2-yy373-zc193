#ifndef cache_hpp
#define cache_hpp

#include <stdio.h>
#include <vector>
#include <pthread.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "Request.h"
#include "response.h"
#include "log.hpp"
#endif /* cache_hpp */
class cache{
public:
    std::vector<std::pair<Request *,Response *> > Cache;
    Log *LOG;
    cache(Log * targetLOG):LOG(targetLOG){
       
    }
  
void Put(Response * Tres, Request * Treq,int thread_id);
    Response * Get(Request *Treq,int fd,int thread_id);
bool isExpire(Response * Tres,int thread_id){
    if(Tres->max_age<0 && Tres->max_age!= -1234567){
        return true;
    }
    time_t now =time(0);
    if(Tres->max_age>0){
        if(Tres->response.find("Date")!=std::string::npos){
            
        if(Tres->max_age+Tres->TDate<=now){
            time_t tempExpire = Tres->max_age + Tres->TDate;
            struct tm * tm = gmtime(&tempExpire);
            std::string Stime=asctime(tm);
            *LOG<< thread_id << ": "<< "in cache, but expired at "<<Stime;
            return true;
        }
        }
    }
    if(Tres->response.find("Expires")!=std::string::npos){
        if(now>Tres->TExpires){
            struct tm * tm = gmtime(&Tres->TExpires);
            std::string Stime=asctime(tm);
            *LOG<< thread_id << ": "<< "in cache, but expired at "<<Stime;
            return true;
        }
    }
    
    return false;
}
~cache(){
    for(std::pair<Request *,Response *> temp :Cache){
    delete temp.first;
    delete temp.second;
    }
}
};
