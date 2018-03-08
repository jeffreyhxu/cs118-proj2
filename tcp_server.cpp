#include "tcp_server.h"

#include <iostream>
#include <string>

#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in

using namespace std;

#include <unistd.h>

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
  unsigned char buf[1024];
  socklen_t addrlen = sizeof(serv_addr);

  while(1) {
    recv_len = recvfrom(serv_fd, buf, 1024, 0,
      (struct sockaddr *) &cli_addr, &addrlen);

    if (recv_len < 0) {
      cerr << "Invalid Message Received" << endl;
      return;
    }

    cout << buf << endl;

    close(cli_fd);
  }
}
