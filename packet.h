#ifndef PACKET_H
#define PACKET_H

#include <vector>

using namespace std;

class Packet {
public:
  // USED TO GENERATE PACKET
  Packet();
  Packet(int ack, int seq, vector<int> flags, char* message);             // used for acks and syns and stuff (unused)
  Packet(int ack, int seq, int len, vector<int> flags, char* message);    // used for data packets

  // USED TO GO FROM PACKET TO HUMAN-READABLE
  Packet(char raw[]);

  int m_ack;
  int m_seq;
  int m_len;
  vector<int> m_flags;      // {ACK, SYN, FIN}
  char* m_message;

  char m_header[8];
  char m_raw[1024];

  /******************************************************
    BITS 0-15 : ACK Number
    BITS 16-31: SEQ Number
    BITS 32-48: MESSAGE LENGTH
    BIT  61   : ACK Flag
    BIT  62   : SYN Flag
    BIT  63   : FIN Flag

    BITS 64-8191: MESSAGE
  *******************************************************/
};

#endif
