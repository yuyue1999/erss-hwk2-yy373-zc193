#include <assert.h>
#include <time.h>
#include <string>
class Response{
public:
    std::string firstLine;
    std::string response;
    int max_age=-1234567;
    time_t TDate;
    time_t TExpires;
    std::string CacheControl;
    std::string Etag;
    std::string LastModified;
    std::string converted_expires;


 Response(std::string response):response(response){
    //max_age
    size_t max_position = response.find("max-age");
    if (max_position!=std::string::npos) {
        size_t equal_position = response.find("=");
        assert(equal_position!=std::string::npos);
        size_t max_end_r = response.find("\r\n", equal_position+1);
        size_t max_end_c = response.find(",", equal_position+1);
        size_t max_end =max_end_r<max_end_c?max_end_r:max_end_c;
        max_age = atoi(response.substr(equal_position+1, max_end-equal_position-1).c_str());
      
    }
     
    size_t first_line = response.find("\r\n");
    firstLine = response.substr(0,first_line);

    //CacheControl
    size_t cache_start = response.find("Cache-Control");
    if (cache_start!=std::string::npos){
      size_t cache_end = response.find("\r\n", cache_start+1);
      CacheControl = response.substr(cache_start, cache_end-cache_start);
      
    }

    //Etag
    size_t etag_position = response.find("ETag");
    if (etag_position!=std::string::npos){
      size_t etag_end = response.find("\r\n", etag_position+1);
      Etag = response.substr(etag_position+6, etag_end-etag_position-6);
    }

    //LastModified
    size_t last_modified_position = response.find("Last-Modified");
    if (last_modified_position!=std::string::npos) {
      size_t last_modified_end = response.find("\r\n",last_modified_position+1);
      LastModified = response.substr(last_modified_position+14, last_modified_end-last_modified_position-14);
      
    }


    //convertedDate
    size_t date_position = response.find("Date");
    if (date_position!=std::string::npos) {
      size_t gmt_position = response.find("GMT", date_position+1);
      std::string converted_date = response.substr(date_position+5, gmt_position-date_position-5);
      tm timeinfo;
      strptime(converted_date.c_str(), " %a, %d %b %Y %H:%M:%S ", &timeinfo);
      TDate = mktime(&timeinfo);
    }
    
    //convertedExpires
    size_t expires_position = response.find("Expires");
    if(expires_position!=std::string::npos){
        size_t gmt1_position = response.find("GMT", expires_position+1);
        if (gmt1_position!=std::string::npos){
            converted_expires = response.substr(expires_position+8, gmt1_position-expires_position-8);
            tm timeinfo1;
            strptime(converted_expires.c_str(), " %a, %d %b %Y %H:%M:%S ", &timeinfo1);
            TExpires = mktime(&timeinfo1);
        }
    }
}
};
