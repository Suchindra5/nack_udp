#ifndef SENDER_H
#define SENDER_H

#include "protocol.h"
#include <map>

class Sender {
private:
    uint32_t next_sequence_number_;
    uint32_t window_base_;
    size_t window_size_;
    std::map<uint32_t, Packet> sent_packets_history_;

public:
    Sender(size_t window_size = 16);
    bool can_send() const;
    bool create_data_packet(const uint8_t* data, size_t length, Packet& out_packet);
    void mark_packet_sent(const Packet& pkt);
    bool get_packet_by_sequence(uint32_t seq, Packet& out_packet) const;
};

#endif // SENDER_H