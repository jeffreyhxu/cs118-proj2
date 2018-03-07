flags = #-Wall -Wextra -Werror

default:
	g++ $(flags) -o server server.cpp
	g++ $(flags) -o client client.cpp
