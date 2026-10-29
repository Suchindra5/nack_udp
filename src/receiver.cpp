#include "receiver.h"

Receiver::Receiver() : expected_sequence_number_(0) {}

void Receiver::process_packet(const Packet& pkt) {
    if (pkt.header.packet_type != static_cast<uint8_t>(PacketType::DATA)) return;

    uint32_t seq = pkt.header.sequence_number;
    received_sequences_.insert(seq);

    // If this packet fills a previously recorded gap, remove it from missing set
    missing_sequences_.erase(seq);

    if (seq == expected_sequence_number_) {
        expected_sequence_number_++;
        // Advance expected sequence past any already-buffered out-of-order arrivals
        while (received_sequences_.count(expected_sequence_number_)) {
            expected_sequence_number_++;
        }
    } else if (seq > expected_sequence_number_) {
        // Gap detected! Record all missing sequence numbers between expected and current
        for (uint32_t i = expected_sequence_number_; i < seq; ++i) {
            if (received_sequences_.find(i) == received_sequences_.end()) {
                missing_sequences_.insert(i);
            }
        }
    }
}

bool Receiver::has_pending_nacks() const {
    return !missing_sequences_.empty();
}

Packet Receiver::generate_aggregated_nack() {
    Packet nack_pkt{};
    nack_pkt.header.packet_type = static_cast<uint8_t>(PacketType::NACK);

    uint32_t count = 0;
    uint32_t* payload_ptr = reinterpret_cast<uint32_t*>(nack_pkt.payload);

    // Pack as many missing sequence numbers as fit into MAX_PAYLOAD_SIZE
    for (auto it = missing_sequences_.begin(); it != missing_sequences_.end() && count < (MAX_PAYLOAD_SIZE / sizeof(uint32_t)); ++it) {
        payload_ptr[count++] = *it;
    }

    nack_pkt.header.ack_or_nack_id = count; // Total count of missing packets in this NACK
    nack_pkt.header.payload_length = count * sizeof(uint32_t);

    return nack_pkt;
}