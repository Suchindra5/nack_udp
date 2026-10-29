#include "sender.h"

Sender::Sender(size_t window_size) 
    : next_sequence_number_(0), window_base_(0), window_size_(window_size) {}

bool Sender::can_send() const {
    return (next_sequence_number_ - window_base_) < window_size_;
}

bool Sender::create_data_packet(const uint8_t* data, size_t length, Packet& out_packet) {
    if (!can_send() || length > MAX_PAYLOAD_SIZE) return false;

    out_packet.header.packet_type = static_cast<uint8_t>(PacketType::DATA);
    out_packet.header.sequence_number = next_sequence_number_;
    out_packet.header.ack_or_nack_id = 0;
    out_packet.header.payload_length = static_cast<uint16_t>(length);

    if (length > 0) {
        std::memcpy(out_packet.payload, data, length);
    }
    return true;
}

void Sender::mark_packet_sent(const Packet& pkt) {
    sent_packets_history_[pkt.header.sequence_number] = pkt;
    next_sequence_number_++;
}

bool Sender::get_packet_by_sequence(uint32_t seq, Packet& out_packet) const {
    auto it = sent_packets_history_.find(seq);
    if (it != sent_packets_history_.end()) {
        out_packet = it->second;
        return true;
    }
    return false;
}