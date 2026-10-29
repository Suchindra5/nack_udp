#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cstdint>
#include <cstring>
#include <vector>

constexpr size_t MAX_PAYLOAD_SIZE = 1400;

enum class PacketType : uint8_t {
    DATA = 1,
    NACK = 2
};

#pragma pack(push, 1)
struct PacketHeader {
    uint8_t packet_type;
    uint32_t sequence_number;
    uint32_t ack_or_nack_id; // Used as count for aggregated NACKs
    uint16_t payload_length;
};

struct Packet {
    PacketHeader header;
    uint8_t payload[MAX_PAYLOAD_SIZE];

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> bytes(sizeof(PacketHeader) + header.payload_length);
        std::memcpy(bytes.data(), &header, sizeof(PacketHeader));
        if (header.payload_length > 0) {
            std::memcpy(bytes.data() + sizeof(PacketHeader), payload, header.payload_length);
        }
        return bytes;
    }

    static bool deserialize(const uint8_t* data, size_t size, Packet& out_packet) {
        if (size < sizeof(PacketHeader)) return false;
        std::memcpy(&out_packet.header, data, sizeof(PacketHeader));
        if (size < sizeof(PacketHeader) + out_packet.payload_length) return false;
        if (out_packet.header.payload_length > MAX_PAYLOAD_SIZE) return false;
        if (out_packet.header.payload_length > 0) {
            std::memcpy(out_packet.payload, data + sizeof(PacketHeader), out_packet.header.payload_length);
        }
        return true;
    }
};
#pragma pack(pop)

#endif // PROTOCOL_H