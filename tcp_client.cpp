#include "tcp_client.h"
#include "packet.h"

#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <map>

#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <arpa/inet.h>
#include <errno.h>

#define MAX_PACKET_SIZE 1024
#define MAX_MSG_SIZE 1016
#define CLI_PORT 9000   // the port that the client socket resides in

using namespace std;

TCP_client::TCP_client() {

}

TCP_client::TCP_client(char* h, unsigned short p, char* fp) {
  hostname = h;
  portnum = p;
  filepath = fp;

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

	map<int, char *> recbuf;
	int lastseq = 0;
	int lastlen = 0; // only need to know length of last message because all previous will be MAX_MSG_SIZE
	while (1) {
		Packet rec;
		receivePacket(rec);
		if (rec.m_flags[2] == 1) { // FIN
			if (rec.m_len != 0) {
				cout << "404 File Not Found" << endl;
			}
			free(rec.m_message);
			vector<int> finflags(3);
			finflags[0] = 1; // ACK
			finflags[2] = 1; // FIN
			char finbuf[MAX_MSG_SIZE] = "FINACK";
			Packet finack(rec.m_seq, 0, 0, finflags, finbuf);
			sendPacket(finack);
			break;
		}
		cout << rec.m_message << endl;

		recbuf[rec.m_seq] = rec.m_message;
		if (rec.m_seq > lastseq) {
			lastseq = rec.m_seq;
			lastlen = rec.m_len;
		}

		vector<int> flags(3);
		flags[0] = 1;
		char buf[MAX_MSG_SIZE] = "ACK";

		Packet ack(rec.m_seq, 0, 0, flags, buf); // ACK num is equal to seq num rather than being the next expected byte. Next expected byte
		sendPacket(ack);                         // lends itself more easily to GBN or the hybrid of GBN and SR used by TCP.
	}

	close(serv_fd);

	// TODO: CONSOLIDATE DATA
	consolidate(recbuf, lastlen, lastseq);

	for (map<int, char *>::iterator it = recbuf.begin(); it != recbuf.end(); ++it) {
		free((*it).second);
	}
}

void TCP_client::handshake() {
  cout << "PREPARING HANDSHAKE" << endl << endl;

  vector<int> flags(3);
  flags[1] = 1;                           //SYN
  char buf[MAX_MSG_SIZE] = "SYN";

  Packet send(0, 0, 0, flags, buf);
  Packet rec;

  while(1) {
    sendPacket(send);

    receivePacket(rec);
	free(rec.m_message);

    if (rec.m_flags[0] != 1 || rec.m_flags[1] != 1) {
      cout << "SYNCHRONIZATION HANDSHAKE FAILED" << endl;
      continue;
    }

    break;
  }

  flags[1] = 0;
  flags[0] = 1;
  strcpy(buf, filepath);

  send = Packet(rec.m_seq + 1, 0, strlen(filepath), flags, buf);
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
  char buf[MAX_PACKET_SIZE];

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
    if (p.m_ack == 0 && p.m_flags[1]) {
      cout << "Sending packet SYN" << endl;
      return;
    }

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

void TCP_client::consolidate(const map<int, char *>& buf, int lastlen, int lastseq) {
	// TODO: write the contents of the buffers in buf into received.data
	ofstream writer("received.data", ios::out | ios::binary);

	for (map<int, char *>::const_iterator it = buf.begin(); it != buf.end(); ++it) {
		writer.write((*it).second, (*it).first == lastseq ? lastlen : MAX_MSG_SIZE);
	}

	writer.close();
}
