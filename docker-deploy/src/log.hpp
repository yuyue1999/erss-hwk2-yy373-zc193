
#include <string>
#include <fstream>
#include <iostream>
class Log {
public:
    std::string File;
    std::ofstream IO;
    std::ios_base::openmode mode;
    Log(std::string target, std::ios_base::openmode tmode):File(target),mode(tmode) {
        IO.open(File.c_str(),mode);
        if (!IO.is_open()) {
            std::cerr << "File does not exist!"<<std::endl;
            exit(EXIT_FAILURE);
        }
    }
    Log& operator << (const std::string & target){
        IO << target;
        IO.flush();
        return *this;
    }
    Log& operator << (const int & target){
        IO << target;
        IO.flush();
        return *this;
    }
    ~Log(){
        if (IO.is_open()) {
            IO.close();
            IO.flush();
        }
    }
};

