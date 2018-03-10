#include "tcp_server.h"
#include "packet.h"

#include <iostream>
#include <string>
#include <vector>

#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in

#include <unistd.h>

#define MAX_PACKET_SIZE 1024        // SPEC
#define MAX_MSG_SIZE 960            // 1024 - 64 FOR TCP HEADER

using namespace std;


TCP_server::TCP_server() {

}

TCP_server::TCP_server(unsigned short p) {
  portnum = p;

  createSocket();
}

TCP_server::~TCP_server() {
  close(serv_fd);
}

void TCP_server::createSocket() {
  cout << "Socket created..." << endl;

  serv_fd = socket(AF_INET, SOCK_DGRAM, 0);  // create socket
  if (serv_fd < 0)
      cerr << "ERROR opening socket" << endl;

  memset((char *) &serv_addr, 0, sizeof(serv_addr));   // reset memory

  // fill in address info
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portnum);

  if (bind(serv_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    cerr << "ERROR on binding: " << strerror(errno) <<  endl;
  }
}

void TCP_server::startServer() {
  cout << "Server started..." << endl;

  int recv_len;
  unsigned char buf[MAX_PACKET_SIZE];
  socklen_t addrlen = sizeof(serv_addr);


  char* buffer = "hello.";
  vector<int> test(3);
  test[0] = 1;

  Packet p(1, 0, test, buffer);

  Packet q(p.m_raw);


  while(1) {
    recv_len = recvfrom(serv_fd, buf, MAX_PACKET_SIZE, 0,
      (struct sockaddr *) &cli_addr, &addrlen);

    if (recv_len < 0) {
      cerr << "Invalid Message Received" << endl;
      return;
    }

    cout << buf << endl;

    close(cli_fd);
  }
}
