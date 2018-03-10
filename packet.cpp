#include "packet.h"

#include <cstring>
#include <stdio.h>

using namespace std;

Packet::Packet(int ack, int seq, vector<int> flags, char* message) {
  m_ack = ack;
  m_seq = seq;
  m_flags = flags;
  m_message = message;

  m_header[0] = (ack & 0xF0) >> 8;
  m_header[1] = (ack & 0x0F);
  m_header[2] = (seq & 0xF0) >> 8;
  m_header[3] = (seq & 0x0F);
  m_header[7] = flags[0] + 2*flags[1] + 4*flags[2];

  strcpy(m_raw, m_header);
  strcpy(m_raw + 8, m_message);
}

Packet::Packet(char raw[]) {
  strcpy(m_raw, raw);

  strcpy(m_header, m_raw);
  strcpy(m_message, m_raw+4);

  m_ack = m_header[0];
  m_seq = m_header[1];

  m_flags[0] = (m_header[3] & 0x01);
  m_flags[1] = (m_header[3] & 0x02) >> 1;
  m_flags[2] = (m_header[3] & 0x04) >> 2;
}
