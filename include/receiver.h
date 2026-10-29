#ifndef RECEIVER_H
#define RECEIVER_H

#include "protocol.h"
#include <set>

class Receiver {
private:
    uint32_t expected_sequence_number_;
    std::set<uint32_t> missing_sequences_;
    std::set<uint32_t> received_sequences_;

public:
    Receiver();
    void process_packet(const Packet& pkt);
    bool has_pending_nacks() const;
    Packet generate_aggregated_nack();
};

#endif // RECEIVER_H