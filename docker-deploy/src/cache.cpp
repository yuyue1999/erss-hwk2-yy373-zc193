#include "cache.hpp"
void cache::Put(Response *Tres, Request *Treq,int thread_id){
    if(Tres->CacheControl.find("no-store")!=std::string::npos || Treq->CacheControl.find("no-store")!=std::string::npos){
    (*LOG)<<thread_id<<": not cacheable because Cache Control is no store"<<"\n";
        return;
    }
    std::pair<Request *,Response *> TempPair(Treq,Tres);
    Cache.push_back(TempPair);
    if(Tres->CacheControl.find("must-revalidate")!=std::string::npos||Tres->CacheControl.find("no-cache")!=std::string::npos||Treq->CacheControl.find("no-cache")\
!=std::string::npos||Tres->CacheControl.find("max-age=0")!=std::string::npos){
        (*LOG)<<thread_id<<": cached, but requires re-validation"<<"\n";
    }
    if(Tres->response.find("Expires:")!=std::string::npos){
      (*LOG)<<thread_id<<": cached, expires at "<<Tres->converted_expires<<"\n";
    }
}

Response * cache::Get(Request *Treq,int fd,int thread_id){
    bool Is_exist=false;
    Response *TempRes;
    int i=0;
    int index=0;
    for(std::pair<Request *,Response *> temp : Cache){
        if(temp.first->uri==Treq->uri){
            index=i;
            Is_exist=true;
            TempRes=temp.second;
            break;
        }
        i++;
    }
    if(Is_exist==false){
      *LOG<< thread_id << ": "<< "not in cache"<<"\n";
        return NULL;
    }
    if(isExpire(TempRes,thread_id)){
        std::pair<Request *,Response *> ReadyDelete=Cache[index];
        Cache.erase(Cache.begin()+index);
        delete ReadyDelete.first;
        delete ReadyDelete.second;
        return NULL;
    }
    bool Needrevalidate=false;
    if(TempRes->CacheControl.find("must-revalidate")!=std::string::npos||TempRes->CacheControl.find("no-cache")!=std::string::npos||Treq->CacheControl.find("no-cache")!=std::string::npos||TempRes->CacheControl.find("max-age=0")!=std::string::npos){
        Needrevalidate=true;
    }
    if(Needrevalidate){
        if(TempRes->Etag=="" && TempRes->LastModified==""){
          *LOG << thread_id << ": "<< "in cache, requires validation"<< "\n";
          return NULL;
        }
        bool hasEtag=false;
        
        if(Treq->request.find("If-None-Match:")==std::string::npos){
            if(TempRes->Etag!=""){
                std::string tempEtag="If-None-Match: "+TempRes->Etag+"\r\n";
                Treq->request=Treq->request.insert(Treq->request.length()-2, tempEtag);
                hasEtag=true;
            }
        }else{
            size_t pos_INM=Treq->request.find("If-None-Match:");
            size_t pos1=Treq->request.find("\"",pos_INM);
            size_t pos2=Treq->request.find("\r\n",pos1+1);
            Treq->request.erase(pos_INM+15,pos2-pos1);
	    Treq->request.insert(pos_INM+15, TempRes->Etag);
            
        }
        
        if(hasEtag==false){
            if(TempRes->LastModified!=""){
                std::string tempLM="If-Modified-Since: "+TempRes->LastModified+"\r\n";
                Treq->request=Treq->request.insert(Treq->request.length()-2, tempLM);
            }
        }
        
        int size=send(fd,Treq->request.c_str(),Treq->request.length()+1,0);
        std::vector<char> store(65536, 0);
        size=recv(fd, store.data(), 65536, 0);
        std::vector<char> temp1=store;
        temp1.push_back('\0');
        std::string tempstring=temp1.data();
        if(tempstring.find("304")==std::string::npos){
            std::pair<Request *,Response *> ReadyDelete=Cache[index];
            Cache.erase(Cache.begin()+index);
            *LOG << thread_id << ": "<< "in cache, requires validation"<< "\n";
            delete ReadyDelete.first;
            delete ReadyDelete.second;
            return NULL;
        }
    }
    *LOG<< thread_id << ": "<< "in cache, valid"<<"\n";
    return TempRes;
    
}
