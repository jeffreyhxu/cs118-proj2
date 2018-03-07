#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in

class TCP_server {
public:
  TCP_server();
  TCP_server(short p);

  ~TCP_server();

private:
  short port;

  int sock_fd;
  struct sockaddr_in serv_addr;

  int cli_fd;
  struct sockaddr_in cli_addr;

};

#endif
