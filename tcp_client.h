#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <string>
#include <map>

#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in

using namespace std;

class Packet;

class TCP_client {
public:
  TCP_client();
  TCP_client(char* h, unsigned short p, char* fp);

  void sendMessage();
  void handshake();

  ~TCP_client();

private:
  void createSocket();
  void sendPacket(Packet p);
  void receivePacket(Packet& p);
  void displayMessage(string dest, Packet p, int wnd = 5120, bool retransmit = false);
  void consolidate(map<int, char *>& buf, int lastlen, int lastseq);

  char* hostname;
  unsigned short portnum;
  char* filepath;

  int serv_fd;
  struct sockaddr_in serv_addr;

  int cli_fd; // unused because we're sending and receiving on same socket
  struct sockaddr_in cli_addr; // unused because the server will discover our address from recvfrom

};

#endif
