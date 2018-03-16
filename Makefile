flags = #-Wall -Wextra -Werror

default:
	g++ $(flags) -o server server.cpp tcp_server.h tcp_server.cpp packet.h packet.cpp -std=c++11 -lrt
	g++ $(flags) -o client client.cpp tcp_client.h tcp_client.cpp packet.h packet.cpp -std=c++11
clean:
	rm server client
