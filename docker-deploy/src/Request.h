#ifndef Request_hpp
#define Request_hpp

#include <stdio.h>

#endif /* Request_hpp */
#include <iostream>
#include <string>
#include <assert.h>

class Request{
  public:
    std::string request;
    std::string method;
    std::string host;
    std::string port;
    std::string uri;
    std::string request_line;
    std::string header;
    std::string body;
    std::string content_len;
    std::string CacheControl;
  public:
    Request(std::string request):request(request){
      
        //method
        size_t method_position = request.find(" ");
        assert (method_position!=std::string::npos);
        method = request.substr(0, method_position);
        
        //host and port
        size_t host_position = request.find("Host");
        assert (host_position!=std::string::npos);
        size_t host_line_end = request.find("\r\n", host_position+1);
        assert (host_line_end!=std::string::npos);
        std::string host_line = request.substr(host_position+6, host_line_end-host_position-6);
        size_t colon_after_host = host_line.find(":");
        if (colon_after_host!=std::string::npos){
          host = host_line.substr(0, colon_after_host);
          port = host_line.substr(colon_after_host+1);
        }
        else {
          host = host_line;
          port = "80";
        }

        //uri
        size_t uri_start = request.find(" ", method_position);
        size_t uri_end = request.find(" ", method_position+1);
        assert (uri_start!=std::string::npos);
        assert (uri_end!=std::string::npos);
        uri = request.substr(uri_start+1, uri_end-uri_start-1);



        //cache control
        size_t cache_start = request.find("Cache-Control");
        if (cache_start!=std::string::npos){
          size_t cache_end = request.find("\r\n", cache_start+1);
          CacheControl = request.substr(cache_start, cache_end-cache_start);
        }
        size_t line_end = request.find("\r\n");
        request_line = request.substr(0,line_end);

        //      parseRequest();

    }
  
};
