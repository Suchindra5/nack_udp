#include "socket.h"
#include "receiver.h"
#include "protocol.h"
#include <iostream>
#include <fstream>
#include <map>
#include <thread>
#include <chrono>

int main() {
    uint16_t port = 8080;
    UdpSocket socket;
    
    if (!socket.bind(port)) {
        std::cerr << "Receiver failed to bind to port " << port << "\n";
        return 1;
    }
    socket.set_non_blocking();

    Receiver receiver;
    std::cout << "[Receiver] Aggregated NACK file-transfer listener active on port " << port << "...\n";

    uint8_t buffer[2048];
    std::string sender_ip;
    uint16_t sender_port;

    std::map<uint32_t, std::vector<uint8_t>> received_chunks;
    bool receiving = true;
    auto start_time = std::chrono::steady_clock::now();

    while (receiving) {
        ssize_t bytes = socket.receive_from(buffer, sizeof(buffer), sender_ip, sender_port);
        if (bytes > 0) {
            start_time = std::chrono::steady_clock::now();
            Packet pkt;
            if (Packet::deserialize(buffer, bytes, pkt)) {
                if (pkt.header.packet_type == static_cast<uint8_t>(PacketType::DATA)) {
                    uint32_t seq = pkt.header.sequence_number;
                    received_chunks[seq] = std::vector<uint8_t>(pkt.payload, pkt.payload + pkt.header.payload_length);

                    receiver.process_packet(pkt);

                    // Send single aggregated NACK if any gaps exist
                    if (receiver.has_pending_nacks()) {
                        Packet aggregated_nack = receiver.generate_aggregated_nack();
                        auto serialized_nack = aggregated_nack.serialize();
                        socket.send_to(serialized_nack.data(), serialized_nack.size(), sender_ip, sender_port);
                        std::cout << "[Receiver] -> Fired Aggregated NACK covering " << aggregated_nack.header.ack_or_nack_id << " missing chunks.\n";
                    }
                }
            }
        } else {
            if (std::chrono::steady_clock::now() - start_time > std::chrono::seconds(3) && !received_chunks.empty()) {
                receiving = false;
            }
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }

    std::ofstream outfile("output.txt", std::ios::binary);
    for (const auto& pair : received_chunks) {
        outfile.write(reinterpret_cast<const char*>(pair.second.data()), pair.second.size());
    }
    outfile.close();

    std::cout << "[Receiver] File saved successfully to 'output.txt'!\n";
    return 0;
}