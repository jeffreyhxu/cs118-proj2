#include "tcp_server.h"
#include "packet.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <cctype>
#include <climits>
#include <algorithm>

#include <stdio.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in

#include <unistd.h>

#define MAX_PACKET_SIZE 1024        // SPEC
#define MAX_MSG_SIZE 1016            // 1024 - 8 FOR TCP HEADER
#define INITIAL_SEQ_NUM 5095

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

	readFile();
  }
}

void TCP_server::handshake() {
  cout << "AWAITING HANDSHAKE" << endl << endl;

  while(1) {
    Packet rec;
    receivePacket(rec);
	free(rec.m_message)

    if (rec.m_flags[1] != 1) continue;

    vector<int> flags(3);
    flags[0] = 1;                           // ACK
    flags[1] = 1;                           // SYN
    char buf[MAX_MSG_SIZE] = "SYNACK";

    Packet send(0, INITIAL_SEQ_NUM, 0, flags, buf);

    sendPacket(send);

    while(1) {
      // TODO: REQUIRES TIMEOUT IMPLEMENTATION
      receivePacket(rec);

      if (rec.m_flags[0] != 1 || rec.m_ack != INITIAL_SEQ_NUM) { // The current implementation of ACK in tcp_client uses the seq num as
        cout << "INVALID RESPONSE TO SYNACK" << endl;            // the ack num instead of its successor, which makes sense if we're not
        continue;                                                // doing the TCP-style blend of GBN and SR.
      }

      filepath = rec.m_message;
	  free(rec.m_message);
      break;
    }

    current_seq = rec.m_ack;
    break;
  }

  cout << endl << "HANDSHAKE SUCCESSFUL" << endl;
}

void TCP_server::readFile() {
	ifstream reader;
	reader.open(filepath.c_str(), ios::in | ios::binary);

	if (!reader) {
		cout << "INVALID FILEPATH" << endl;
		// TODO: 404 NOT FOUND MESSAGE?
	}

	char *buf = new char[MAX_MSG_SIZE];
	bool going = true;
	while (going) {
		reader.read(buf, MAX_MSG_SIZE);
		int len = MAX_MSG_SIZE;
		if (!reader) {
			len = (int)reader.gcount();
			going = false;
		}
		vector<int> flags(3);
		Packet send(0, current_seq, MAX_MSG_SIZE, flags, buf);
		sendPacket(send);

		Packet rec;
		receivePacket(rec);
		free(rec.m_message);
		current_seq += len;
		// TODO: IMPLEMENT SELECTIVE REPEAT (MAY NEED POLLING)
	}
	// FIN
	vector<int> finflags(3);
	finflags[2] = 1;
	char finbuf[MAX_MSG_SIZE] = "FIN";
	Packet fin(0, current_seq, 0, finflags, finbuf);
	sendPacket(fin);
	// TODO: TIMED WAIT FOR FINACK (MAYBE CLIENT SIDE?)
	reader.close();
}

void TCP_server::sendPacket(Packet p) {
  displayMessage("sending", p);
  sendto(serv_fd, p.m_raw, MAX_PACKET_SIZE, 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
}

void TCP_server::receivePacket(Packet& p) {
  int recv_len;
  socklen_t addrlen = sizeof(cli_addr);
  char buf[MAX_PACKET_SIZE];

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
