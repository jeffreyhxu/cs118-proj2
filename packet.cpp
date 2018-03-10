#include "packet.h"

#include <iostream>
#include <cstring>
#include <stdio.h>

using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <vector>

Packet::Packet() {

}

Packet::Packet(int ack, int seq, int len, vector<int> flags, char* message) {
  m_ack = ack;
  m_seq = seq;
  m_len = len;
  m_flags = flags;
  m_message = message;

  m_header[0] = (ack & 0xF0) >> 8;
  m_header[1] = (ack & 0x0F);
  m_header[2] = (seq & 0xF0) >> 8;
  m_header[3] = (seq & 0x0F);
  m_header[4] = (len & 0xF0) >> 8;
  m_header[5] = (len & 0x0F);
  m_header[7] = flags[0] + 2*flags[1] + 4*flags[2];

  memcpy(m_raw, m_header, 8);
  memcpy(m_raw + 8, m_message, 120);
}

Packet::Packet(char raw[]) {
  memcpy(m_raw, raw, 128);

  memcpy(m_header, m_raw, 8);

  m_message = (char*)malloc(120);
  memcpy(m_message, m_raw+8, 120);

  m_ack = m_header[0] << 8 + m_header[1];
  m_seq = m_header[2] << 8 + m_header[3];
  m_len = m_header[4] << 8 + m_header[5];

  m_flags.resize(8);
  m_flags[0] = (m_header[7] & 0x01);
  m_flags[1] = (m_header[7] & 0x02) >> 1;
  m_flags[2] = (m_header[7] & 0x04) >> 2;
}
