flags = #-Wall -Wextra -Werror

default: server client

server: server.cpp tcp_server.h tcp_server.cpp packet.h packet.cpp
	g++ $(flags) -o server server.cpp tcp_server.h tcp_server.cpp packet.h packet.cpp -std=c++11 -lrt
client: client.cpp tcp_client.h tcp_client.cpp packet.h packet.cpp
	g++ $(flags) -o client client.cpp tcp_client.h tcp_client.cpp packet.h packet.cpp -std=c++11 -lrt
clean:
	rm -f server client
