#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <fstream>
#include <string>
#include <queue>
#include <ctime>

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
  void readFile();
  void sendPacket(Packet p, int wnd = 5120, bool retransmit = false);
  void receivePacket(Packet& p);
  void displayMessage(string dest, Packet p, int wnd = 5120, bool retransmit = false);
  long long timeSince(struct timespec then);

  unsigned short portnum;
  string filepath;
  int current_seq;
  queue<Packet *> unacked; // sorted by time since last (re)transmission
  queue<struct timespec> sendtime; // time last sent or resent, should stay synced to unacked
  int first_seq; // lowest seq num in the window

  int serv_fd;
  struct sockaddr_in serv_addr;

  int cli_fd; // unused, remove
  struct sockaddr_in cli_addr; // unused, remove

};

#endif
