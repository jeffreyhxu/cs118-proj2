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
  handshake();

  char buf[1024];
  strcpy(buf, "TESTING SENT MESSAGE\0");

  vector<int> test(3);
  test[0] = 1;

  Packet p(0, 0, 0, test, buf);
  //sendto(serv_fd, buf, (strlen(buf) + 1), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

}

void TCP_client::handshake() {
  cout << "PREPARING HANDSHAKE" << endl << endl;

  vector<int> flags(3);
  flags[1] = 1;                           //SYN
  char buf[1024] = "SYN";

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

  flags[1] = 0;
  flags[0] = 1;
  strcpy(buf, "ACK");

  send = Packet(0, 0, 0, flags, buf);
  sendPacket(send);

  cout << endl << "HANDSHAKE SUCCESSFUL" << endl;
}

void TCP_client::sendPacket(Packet p) {
  displayMessage("sending", p);
  sendto(serv_fd, p.m_raw, 1024, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
}

void TCP_client::receivePacket(Packet& p) {
  int recv_len;
  socklen_t addrlen = sizeof(serv_addr);
  char buf[1024];

  recv_len = recvfrom(serv_fd, buf, MAX_PACKET_SIZE, 0,
    (struct sockaddr *) &serv_addr, &addrlen);

  if (recv_len < 0) {
    cerr << "Invalid Message Received" << endl;
    return;
  }

  p = Packet(buf);
  displayMessage("receiving", p);
}

void TCP_client::displayMessage(string dest, Packet p, int wnd, bool retransmit) {
  if (dest == "sending") {
    cout << "Sending packet " << p.m_ack << " " << wnd << " ";

    if (retransmit) cout << "Retransmission" << " ";
    if (p.m_flags[1]) cout << "SYN" << " ";
    if (p.m_flags[2]) cout << "FIN" << " ";
    cout << endl;
  }
  else if (dest == "receiving") {
    cout << "Receiving packet " << p.m_seq << endl;
  }
  else {
    cerr << "Invalid dest in displayMessage" << endl;
  }
}
