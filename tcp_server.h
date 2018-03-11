#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <string>

#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in

class Packet;

using namespace std;

class TCP_server {
public:
  TCP_server();
  TCP_server(unsigned short p);

  void startServer();
  void handshake();

  ~TCP_server();

private:
  void createSocket();
  void sendPacket(Packet p);
  void receivePacket(Packet& p);
  void displayMessage(string dest, Packet p, int wnd = 5120, bool retransmit = false);

  unsigned short portnum;
  string filepath;

  int serv_fd;
  struct sockaddr_in serv_addr;

  int cli_fd;
  struct sockaddr_in cli_addr;

};

#endif
