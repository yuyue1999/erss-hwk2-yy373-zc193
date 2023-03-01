#include "Proxy.hpp"

void Proxy::ConnectRequest(){
    std::string message("HTTP/1.1 200 OK\r\n\r\n");
    send(browser_fd, message.c_str() , message.size(), MSG_NOSIGNAL);
    fd_set read_fds;
    int fd_max=0;
    if(browser_fd>originalServer_fd){
        fd_max=browser_fd;
    }else{
        fd_max=originalServer_fd;
    }
    while(true){
        FD_ZERO(&read_fds);
        FD_SET(browser_fd, &read_fds);
        FD_SET(originalServer_fd, &read_fds);
        int select_status=select(fd_max+ 1,&read_fds, NULL, NULL, NULL);
        if(select_status == -1) {
            std::cerr<<"Cannot select"<<std::endl;
            return;
        }
        std::vector<int> FD_Set;
        FD_Set.push_back(browser_fd);
        FD_Set.push_back(originalServer_fd);
        int recv_size;
        int send_size;
        for(int i=0;i<2;i++){
            std::vector<char> temp_data(131072,0);
            if(FD_ISSET(FD_Set[i], &read_fds)){
                int j=0;
                if(i==0){
                    j=1;
                }else{
                    j=0;
                }
                recv_size = recv(FD_Set[i], temp_data.data(), temp_data.size(), MSG_NOSIGNAL);
                if(recv_size <= 0){
                    return;
                }
                send_size = send(FD_Set[j], temp_data.data(), recv_size, MSG_NOSIGNAL);
                if(send_size <=0){
                    return;
                }
    
            }
            
        }
        
    }
    
}

bool Proxy::isContentEnding(std::vector<char> &store,int Clength){
    std::vector<char> temp=store;
    temp.push_back('\0');
    std::string target=temp.data();
    size_t headerEnd=target.find("\r\n\r\n");
    std::vector<char> body;
    for(size_t i=headerEnd+4;i<store.size();i++){
        body.push_back(store[i]);
    }
    if(body.size()<Clength){
        return false;
    }
    return true;
}
Response* Proxy::GetRequest(){
    std::vector<char> store;
    std::vector<char> first(65536,0);
    int size=recv(originalServer_fd, first.data(), first.size(), 0);
    for(int i = 0; i < size;i++){
        store.push_back(first[i]);
    }
    std::vector<char> temp=first;
    temp.push_back('\0');
    std::string tempstring=temp.data();
    if(tempstring.find("Content-Length")!=std::string::npos){
        size_t posOfCL=tempstring.find("Content-Length: ");
        size_t posOfn=tempstring.find("\n",posOfCL);
        std::string Clengths=tempstring.substr(posOfCL+16,posOfn-posOfCL-16);
        int Clength = std::stoi (Clengths,nullptr);
        if(!isContentEnding(store, Clength)){
            while(true){
                std::vector<char> temp_store(32768,0);
                size=recv(originalServer_fd, temp_store.data(), temp_store.size(), 0);
                if(size==0){
                    break;
                }
                for(int i = 0; i < size;i++){
                    store.push_back(temp_store[i]);
                }
                std::vector<char> temp=store;
                temp.push_back('\0');
                std::string tempstring=temp.data();
                if(isContentEnding(store, Clength)){
                    break;
                }
            }
        }
            std::string resp(store.begin(), store.end());
            Response * Res=new Response(resp);
            if(resp.find("\r\n\r\n") == std::string::npos){
                std::string bad_gateway = "HTTP/1.1 502 Bad Gateway";
                send(browser_fd, bad_gateway.c_str(), bad_gateway.size(), 0);
                return NULL;
            }
            send(browser_fd, store.data(), store.size(), MSG_NOSIGNAL);
            return Res;
        }else if(tempstring.find("Transfer-Encoding")!= std::string::npos &&
                 tempstring.find("chunked")!= std::string::npos){
            while(true){
                std::vector<char> temp_store(65536,0);
                size=recv(originalServer_fd, temp_store.data(), temp_store.size(), 0);
                if(size==0){
                    break;
                }
                for(int i = 0; i < size;i++){
                    store.push_back(temp_store[i]);
                }
                std::vector<char> temp=temp_store;
                temp.push_back('\0');
                tempstring=temp.data();
                if(tempstring.find("0\r\n\r\n")!=std::string::npos){
                    break;
                }
            }
            std::string resp(store.begin(), store.end());
            Response * Res=new Response(resp);
            if(resp.find("\r\n\r\n") == std::string::npos){
                std::string bad_gateway = "HTTP/1.1 502 Bad Gateway";
                send(browser_fd, bad_gateway.c_str(), bad_gateway.size(), 0);
                return NULL;
            }
            send(browser_fd, store.data(), store.size(), MSG_NOSIGNAL);
            return Res;
        }else if(tempstring.find("304")!=std::string::npos && tempstring.find("Not Modified")!=std::string::npos){
            std::string resp(store.begin(), store.end());
            Response * Res=new Response(resp);
            send(browser_fd, store.data(), store.size(), MSG_NOSIGNAL);
            return Res;
        }else if(tempstring.find("301")!=std::string::npos && tempstring.find("Moved Permanently")!=std::string::npos){
            std::string resp(store.begin(), store.end());
            Response * Res=new Response(resp);
            send(browser_fd, store.data(), store.size(), MSG_NOSIGNAL);
            return Res;
        }else if(tempstring.find("302")!=std::string::npos && tempstring.find("Found")!=std::string::npos){
            std::string resp(store.begin(), store.end());
            Response * Res=new Response(resp);
            send(browser_fd, store.data(), store.size(), MSG_NOSIGNAL);
            return Res;
        }else if(tempstring.find("303")!=std::string::npos && tempstring.find("See Other")!=std::string::npos){
            std::string resp(store.begin(), store.end());
            Response * Res=new Response(resp);
            send(browser_fd, store.data(), store.size(), MSG_NOSIGNAL);
            return Res;
        }else if(tempstring.find("307")!=std::string::npos && tempstring.find("Temporary Redirect")!=std::string::npos){
            std::string resp(store.begin(), store.end());
            Response * Res=new Response(resp);
            send(browser_fd, store.data(), store.size(), MSG_NOSIGNAL);
            return Res;
        }else if(tempstring.find("308")!=std::string::npos && tempstring.find("Permanent Redirect")!=std::string::npos){
            std::string resp(store.begin(), store.end());
            Response * Res=new Response(resp);
            send(browser_fd, store.data(), store.size(), MSG_NOSIGNAL);
            return Res;
        }else if(tempstring.find("200")!=std::string::npos && tempstring.find("OK")!=std::string::npos){
            std::string resp(store.begin(), store.end());
            Response * Res=new Response(resp);
            send(browser_fd, store.data(), store.size(), MSG_NOSIGNAL);
            return Res;
        }else{
            std::string resp(store.begin(), store.end());
            Response * Res=new Response(resp);
            send(browser_fd, store.data(), store.size(), MSG_NOSIGNAL);
            return Res;
        }



    return NULL;
}

Response * Proxy::PostRequest(){
    std::vector<char> store;
    std::vector<char> first(65536,0);
    int size=recv(originalServer_fd, first.data(), first.size(), 0);
    for(int i = 0; i < size;i++){
        store.push_back(first[i]);
    }
    std::vector<char> temp=first;
    temp.push_back('\0');
    std::string tempstring=temp.data();
    if(tempstring.find("Content-Length")!=std::string::npos){
        size_t posOfCL=tempstring.find("Content-Length: ");
        size_t posOfn=tempstring.find("\n",posOfCL);
        std::string Clengths=tempstring.substr(posOfCL+16,posOfn-posOfCL-16);
        int Clength = std::stoi (Clengths,nullptr);
        if(!isContentEnding(store, Clength)){
            while(true){
                std::vector<char> temp_store(32768,0);
                
                size=recv(originalServer_fd, temp_store.data(), temp_store.size(), 0);
                
                if(size==0){
                    break;
                }
                for(int i = 0; i < size;i++){
                    store.push_back(temp_store[i]);
                }
                std::vector<char> temp=store;
                temp.push_back('\0');
                std::string tempstring=temp.data();
                if(isContentEnding(store, Clength)){
                        break;
                }
            }
        }
            std::string resp(store.begin(), store.end());
            Response * Res=new Response(resp);
            if(resp.find("\r\n\r\n") == std::string::npos){
                std::string bad_gateway = "HTTP/1.1 502 Bad Gateway";
                send(browser_fd, bad_gateway.c_str(), bad_gateway.size(), 0);
                return NULL;
            }
            send(browser_fd, store.data(), store.size(), MSG_NOSIGNAL);
            return Res;
        }else if(tempstring.find("Transfer-Encoding")!= std::string::npos &&
                 tempstring.find("chunked")!= std::string::npos){
            while(true){
                std::vector<char> temp_store(65536,0);
                size=recv(originalServer_fd, temp_store.data(), temp_store.size(), 0);
                if(size==0){
                    break;
                }
                for(int i = 0; i < size;i++){
                    store.push_back(temp_store[i]);
                }
                std::vector<char> temp=temp_store;
                temp.push_back('\0');
                tempstring=temp.data();
                if(tempstring.find("0\r\n\r\n")!=std::string::npos){
                    break;
                }
            }
            std::string resp(store.begin(), store.end());
            Response * Res=new Response(resp);
            if(resp.find("\r\n\r\n") == std::string::npos){
                std::string bad_gateway = "HTTP/1.1 502 Bad Gateway";
                send(browser_fd, bad_gateway.c_str(), bad_gateway.size(), 0);
                return NULL;
            }
            send(browser_fd, store.data(), store.size(), MSG_NOSIGNAL);
            return Res;
        }else if(tempstring.find("304")!=std::string::npos && tempstring.find("Not Modified")!=std::string::npos){
            std::string resp(store.begin(), store.end());
            Response * Res=new Response(resp);
            send(browser_fd, store.data(), store.size(), MSG_NOSIGNAL);
            return Res;
        }else if(tempstring.find("301")!=std::string::npos && tempstring.find("Moved Permanently")!=std::string::npos){
            std::string resp(store.begin(), store.end());
            Response * Res=new Response(resp);
            send(browser_fd, store.data(), store.size(), MSG_NOSIGNAL);
            return Res;
        }else if(tempstring.find("302")!=std::string::npos && tempstring.find("Found")!=std::string::npos){
            std::string resp(store.begin(), store.end());
            Response * Res=new Response(resp);
            send(browser_fd, store.data(), store.size(), MSG_NOSIGNAL);
            return Res;
        }else if(tempstring.find("303")!=std::string::npos && tempstring.find("See Other")!=std::string::npos){
            std::string resp(store.begin(), store.end());
            Response * Res=new Response(resp);
            send(browser_fd, store.data(), store.size(), MSG_NOSIGNAL);
            return Res;
        }else if(tempstring.find("307")!=std::string::npos && tempstring.find("Temporary Redirect")!=std::string::npos){
            std::string resp(store.begin(), store.end());
            Response * Res=new Response(resp);
            send(browser_fd, store.data(), store.size(), MSG_NOSIGNAL);
            return Res;
        }else if(tempstring.find("308")!=std::string::npos && tempstring.find("Permanent Redirect")!=std::string::npos){
            std::string resp(store.begin(), store.end());
            Response * Res=new Response(resp);
            send(browser_fd, store.data(), store.size(), MSG_NOSIGNAL);
            return Res;
        }else if(tempstring.find("200")!=std::string::npos && tempstring.find("OK")!=std::string::npos){
            std::string resp(store.begin(), store.end());
            Response * Res=new Response(resp);
            send(browser_fd, store.data(), store.size(), MSG_NOSIGNAL);
            return Res;
        }else{
            std::string resp(store.begin(), store.end());
            Response * Res=new Response(resp);
            send(browser_fd, store.data(), store.size(), MSG_NOSIGNAL);
            return Res;
        }

    return NULL;
}
