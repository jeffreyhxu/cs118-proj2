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
#include <queue>
#include <ctime>

#include <poll.h>
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
#define WINDOW_SIZE 5120

using namespace std;


TCP_server::TCP_server() {

}

TCP_server::TCP_server(unsigned short p) {
  portnum = p;

  createSocket();
}

TCP_server::~TCP_server() {
  if (serv_fd > 0) {
	  close(serv_fd);
  }
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
    free(rec.m_message);

    if (rec.m_flags[1] != 1) continue;

    vector<int> flags(3);
    flags[0] = 1;                           // ACK
    flags[1] = 1;                           // SYN
    char buf[MAX_MSG_SIZE] = "SYNACK";      // not strictly necessary but useful for readability

    Packet send(0, INITIAL_SEQ_NUM, 0, flags, buf);

    sendPacket(send);
    struct timespec syntime;
	clock_gettime(CLOCK_MONOTONIC, &syntime);

    struct pollfd fds[1]; // to poll for ACKs
    fds[0].fd = serv_fd;
    fds[0].events = POLLIN;

    while(1) {
      if (poll(fds, 1, 1) > 0) { // poll for ACKs but don't wait up so we can keep checking the time
        receivePacket(rec);

        if (rec.m_flags[0] != 1 || rec.m_ack != INITIAL_SEQ_NUM) { // The current implementation of ACK in tcp_client uses the seq num as
          cout << "INVALID RESPONSE TO SYNACK" << endl;            // the ack num instead of its successor, which makes sense if we're not
          continue;                                                // doing the TCP-style blend of GBN and SR.
        }

        filepath = rec.m_message; // this isn't a pointer but a constructor for a string
        free(rec.m_message);      // so this isn't as scary as it looks
        break;
      }


      if (timeSince(syntime) >= 500) {
        sendPacket(send, WINDOW_SIZE, true);
        clock_gettime(CLOCK_MONOTONIC, &syntime);
      }
    }

    current_seq = rec.m_ack + 8; // 8 byte header
    first_seq = INT_MAX;
    break;
  }

  cout << endl << "HANDSHAKE SUCCESSFUL" << endl;
}

void TCP_server::readFile() {
  ifstream reader;
  reader.open(filepath.c_str(), ios::in | ios::binary);

  struct pollfd fds[1]; // to poll for ACKs
  fds[0].fd = serv_fd;
  fds[0].events = POLLIN;
  
  if (!reader) {
    cout << "INVALID FILEPATH" << endl;
    char buf404[MAX_MSG_SIZE] = "404";  // 404 will be signified by a FIN packet with non-zero length.
    vector<int> flags404(3);
    flags404[1] = 1;
    Packet send404(0, current_seq, 4, flags404, buf404);
    sendPacket(send404);
    
    struct timespec time404;
    clock_gettime(CLOCK_MONOTONIC, &time404);
    while (1) {
      if (poll(fds, 1, 0) > 0) { // check to see if a FINACK has arrived but don't wait up
        Packet rec;
        receivePacket(rec);
        free(rec.m_message);
        break;
      }
      if (timeSince(time404) >= 1000) {
        sendPacket(send404, WINDOW_SIZE, true);
        clock_gettime(CLOCK_MONOTONIC, &time404);
      }
    }
    reader.close();
    return;
  }

  char buf[MAX_MSG_SIZE];
  bool filedone = false;
  bool hasbuf = false;
  
  while (!filedone || !unacked.empty()) {
    int len = MAX_MSG_SIZE;
    if (!hasbuf) { // prepare the next block of data
      reader.read(buf, MAX_MSG_SIZE);
      if (!reader) {
	len = (int)reader.gcount();
	cout << "lastlen: " << len << endl;
	filedone = true;
      }
      hasbuf = true;
    }

    if (current_seq + len + 8 - first_seq <= WINDOW_SIZE) { // if space in the window, send that block of data and start its timer
      vector<int> flags(3);
      Packet *send = new Packet(0, current_seq, len, flags, buf);
      sendPacket(*send);
      
      unacked.push(send);
      struct timespec sent;
	  clock_gettime(CLOCK_MONOTONIC, &sent);
      sendtime.push(sent);
      first_seq = min(first_seq, current_seq);
      current_seq += len + 8;
      hasbuf = false;
    }
    
    if (poll(fds, 1, 1) > 0) { // check to see if an ACK has arrived but don't wait up
      Packet rec;
      receivePacket(rec);
      free(rec.m_message);
      
      first_seq = INT_MAX;
      for (int i = 0; i < unacked.size(); i++) { // remove the acked packet from the window and update first_seq
        if (unacked.front()->m_seq == rec.m_ack) {
          delete unacked.front();
          unacked.pop();
          sendtime.pop();
        }
        else {
          first_seq = min(first_seq, unacked.front()->m_seq);
          unacked.push(unacked.front());
          unacked.pop();
          sendtime.push(sendtime.front());
        }
      }
    }

    if (unacked.empty()) {
      continue;
    }
    
    while (timeSince(sendtime.front()) >= 500) { // resend all timed-out packets, starting with oldest
      Packet *resend = unacked.front();
      unacked.pop();
      sendPacket(*resend, WINDOW_SIZE, true);
      unacked.push(resend);
      sendtime.pop();
      struct timespec newsent;
	  clock_gettime(CLOCK_MONOTONIC, &newsent);
      sendtime.push(newsent);
    }
  }
  // FIN
  vector<int> finflags(3);
  finflags[2] = 1;
  char finbuf[MAX_MSG_SIZE] = "FIN";
  Packet fin(0, current_seq, 0, finflags, finbuf);
  sendPacket(fin);
  struct timespec fintime;
  clock_gettime(CLOCK_MONOTONIC, &fintime);
  while (1) {
    if (poll(fds, 1, 1) > 0) { // check to see if a FINACK has arrived but don't wait up
      Packet rec;
      receivePacket(rec);
      free(rec.m_message);
      break;
    }
    if (timeSince(fintime) >= 1000) {
      sendPacket(fin);
      clock_gettime(CLOCK_MONOTONIC, &fintime);
    }
  }
  reader.close();
}

void TCP_server::sendPacket(Packet p, int wnd, bool retransmit) {
  displayMessage("sending", p, wnd, retransmit);
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

long long TCP_server::timeSince(struct timespec then) {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return ((now.tv_sec - then.tv_sec) * 1000000000 + now.tv_nsec - then.tv_nsec) / 1000000;
}