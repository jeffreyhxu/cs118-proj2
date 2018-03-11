#include "tcp_server.h"
#include "packet.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <cctype>

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
  while(1) {
    handshake();



  }

  // TODO: OPEN FILE, DIVIDE INTO SUBPACKETS, AND SEND EACH PACKET
  // TODO: IMPLEMENT SELECTIVE REPEAT
}

void TCP_server::handshake() {
  cout << "AWAITING HANDSHAKE" << endl << endl;

  while(1) {
    Packet rec;
    receivePacket(rec);

    if (rec.m_flags[1] != 1) continue;

    vector<int> flags(3);
    flags[0] = 1;                           // ACK
    flags[1] = 1;                           // SYN
    char buf[1024] = "SYNACK";

    Packet send(0, 5095, 0, flags, buf);

    sendPacket(send);

    while(1) {
      // TODO: REQUIRES TIMEOUT IMPLEMENTATION
      receivePacket(rec);

      if (rec.m_flags[0] != 1 || rec.m_ack != 5096) {
        cout << "INVALID RESPONSE TO SYNACK" << endl;
        continue;
      }

      filepath = rec.m_message;
      break;
    }

    break;
  }

  cout << endl << "HANDSHAKE SUCCESSFUL" << endl;
}

void TCP_server::sendPacket(Packet p) {
  displayMessage("sending", p);
  sendto(serv_fd, p.m_raw, 1024, 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
}

void TCP_server::receivePacket(Packet& p) {
  int recv_len;
  socklen_t addrlen = sizeof(cli_addr);
  char buf[1024];

  recv_len = recvfrom(serv_fd, buf, MAX_PACKET_SIZE, 0,
    (struct sockaddr *) &cli_addr, &addrlen);

  if (recv_len < 0) {
    cerr << "Invalid Message Received" << endl;
    return;
  }

  p = Packet(buf);
  displayMessage("receiving", p);
}

void TCP_server::displayMessage(string dest, Packet p, int wnd, bool retransmit) {
  if (dest == "sending") {
    cout << "Sending packet " << p.m_seq << " " << wnd << " ";

    if (retransmit) cout << "Retransmission" << " ";
    if (p.m_flags[1]) cout << "SYN" << " ";
    if (p.m_flags[2]) cout << "FIN" << " ";
    cout << endl;
  }
  else if (dest == "receiving") {
    cout << "Receiving packet " << p.m_ack << endl;
  }
  else {
    cerr << "Invalid dest in displayMessage" << endl;
  }
}
