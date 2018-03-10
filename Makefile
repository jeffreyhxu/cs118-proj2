flags = #-Wall -Wextra -Werror

default:
	g++ $(flags) -o server server.cpp tcp_server.h tcp_server.cpp packet.h packet.cpp
	g++ $(flags) -o client client.cpp tcp_client.h tcp_client.cpp
