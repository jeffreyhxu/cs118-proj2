#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <vector>

using namespace std;

class Packet {
public:
  // USED TO GENERATE PACKET
  Packet(int ack, int seq, vector<int> flags, char* message);

  // USED TO GO FROM PACKET TO HUMAN-READABLE
  Packet(char raw[]);

  int m_ack;
  int m_seq;
  vector<int> m_flags;      // {ACK, SEQ, FIN}
  char* m_message;

  char m_header[8];
  char m_raw[128];

  /******************************************************
    BITS 0-15 : ACK Number
    BITS 16-32: SEQ Number
    BIT  61   : ACK Flag
    BIT  62   : SYN Flag
    BIT  63   : FIN Flag

    BITS 64-1024: MESSAGE
  *******************************************************/
};

#endif
