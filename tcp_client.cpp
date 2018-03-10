#include "tcp_client.h"
#include "packet.h"

#include <iostream>
#include <cstring>
#include <vector>

#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <arpa/inet.h>
#include <errno.h>

#define MAX_PACKET_SIZE 1024
#define CLI_PORT 9000   // the port that the client socket resides in

using namespace std;

TCP_client::TCP_client() {

}

TCP_client::TCP_client(char* h, unsigned short p) {
  hostname = h;
  portnum = p;

  createOwnSocket();
  createSocket();
}

TCP_client::~TCP_client() {

}

void TCP_client::createOwnSocket() {
  cout << "Client (personal) socket created..." << endl;

  cli_fd = socket(AF_INET, SOCK_DGRAM, 0);  // create socket
  if (cli_fd < 0)
      cerr << "ERROR opening socket" << endl;

  memset((char *) &cli_addr, 0, sizeof(cli_addr));   // reset memory

  // fill in address info
  cli_addr.sin_family = AF_INET;
  cli_addr.sin_addr.s_addr = INADDR_ANY;
  cli_addr.sin_port = htons(CLI_PORT);

  if (bind(cli_fd, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0) {
    cerr << "ERROR on binding: " << strerror(errno) <<  endl;
  }
}

void TCP_client::createSocket() {
  cout << "Server socket created..." << endl;

  serv_fd = socket(AF_INET, SOCK_DGRAM, 0);  // create socket
  if (serv_fd < 0)
      cerr << "ERROR opening socket" << endl;

  memset((char *) &serv_addr, 0, sizeof(serv_addr));   // reset memory

  // fill in address info
  serv_addr.sin_family = AF_INET;
  inet_aton(hostname, &serv_addr.sin_addr);
  serv_addr.sin_port = htons(portnum);
}

void TCP_client::sendMessage() {
  handshake();

  char buf[1024];
  strcpy(buf, "TESTING SENT MESSAGE\0");

  vector<int> test(3);
  test[0] = 1;

  Packet p(1, 0, 0, test, buf);

  //sendto(serv_fd, buf, (strlen(buf) + 1), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

}

void TCP_client::handshake() {
  vector<int> flags(3);
  flags[1] = 1;                           //SYN
  char buf[1024] = "SENDING SYN";

  Packet send(0, 0, 0, flags, buf);
  while(1) {
    sendPacket(send);

    Packet rec;
    receivePacket(rec);

    if (rec.m_flags[0] != 1 || rec.m_flags[1] != 1) {
      cout << "SYNCHRONIZATION HANDSHAKE FAILED" << endl;
      continue;
    }

    break;
  }

  cout << "HANDSHAKE SUCCESSFUL" << endl;
}

void TCP_client::sendPacket(Packet p) {
  sendto(serv_fd, p.m_raw, 1024, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
}

void TCP_client::receivePacket(Packet& p) {
  int recv_len;
  socklen_t addrlen = sizeof(serv_addr);
  char buf[1024];

  recv_len = recvfrom(cli_fd, buf, MAX_PACKET_SIZE, 0,
    (struct sockaddr *) &serv_addr, &addrlen);

  if (recv_len < 0) {
    cerr << "Invalid Message Received" << endl;
    return;
  }

  p = Packet(buf);
  cout << p.m_message << endl;
}
