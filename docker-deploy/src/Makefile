all: main

main: main.cpp Request.h socket.cpp socket.hpp Proxy.cpp Proxy.hpp cache.cpp cache.hpp response.h log.hpp
	g++ -g -std=c++11 -o main main.cpp Request.h socket.cpp socket.hpp Proxy.cpp Proxy.hpp cache.cpp cache.hpp response.h log.hpp -pthread

.PHONY:
	clean
clean:
	rm -rf *.o main
