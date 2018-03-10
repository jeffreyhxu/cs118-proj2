#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in

class Packet;

class TCP_client {
public:
  TCP_client();
  TCP_client(char* h, unsigned short p);

  void sendMessage();
  void handshake();

  ~TCP_client();

private:
  void createOwnSocket();
  void createSocket();
  void sendPacket(Packet p);
  void receivePacket(Packet& p);

  char* hostname;
  unsigned short portnum;

  int serv_fd;
  struct sockaddr_in serv_addr;

  int cli_fd;
  struct sockaddr_in cli_addr;

};

#endif
