#include "tcp_client.h"

#include <iostream>
#include <cstring>

#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <arpa/inet.h>

using namespace std;

TCP_client::TCP_client() {

}

TCP_client::TCP_client(char* h, unsigned short p) {
  hostname = h;
  portnum = p;

  createSocket();
}

TCP_client::~TCP_client() {

}

void TCP_client::createSocket() {
  cout << "Socket created..." << endl;

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
  char buf[1024];
  strcpy(buf, "TESTING SENT MESSAGE\0");

  sendto(serv_fd, buf, (strlen(buf) + 1), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));


}
