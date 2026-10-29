#include "socket.h"
#include "sender.h"
#include "loss_emulator.h"
#include "protocol.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>

int main() {
    UdpSocket socket;
    Sender sender(16); 
    LossEmulator loss_emulator(0.25); // 25% loss rate

    std::string receiver_ip = "127.0.0.1";
    uint16_t receiver_port = 8080;

    std::string filename = "input.txt";
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "[Error] Could not open file " << filename << "\n";
        return 1;
    }
    
    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::cout << "\n==================================================\n";
    std::cout << "      AGGREGATED NACK FILE STREAMING PROTOCOL     \n";
    std::cout << "==================================================\n";
    std::cout << "[*] Streaming file: " << filename << " (" << file_size << " bytes)\n";

    std::vector<std::vector<uint8_t>> file_chunks;
    char read_buffer[MAX_PAYLOAD_SIZE];
    while (file.read(read_buffer, MAX_PAYLOAD_SIZE) || file.gcount() > 0) {
        std::streamsize bytes_read = file.gcount();
        file_chunks.push_back(std::vector<uint8_t>(read_buffer, read_buffer + bytes_read));
    }
    file.close();

    size_t total_packets = file_chunks.size();
    size_t sent_count = 0;
    int dropped_count = 0;
    int retransmitted_count = 0;

    uint8_t net_buffer[2048];
    std::string from_ip;
    uint16_t from_port;

    while (sent_count < total_packets) {
        if (sender.can_send() && sent_count < total_packets) {
            const auto& chunk = file_chunks[sent_count];
            Packet pkt;
            if (sender.create_data_packet(chunk.data(), chunk.size(), pkt)) {
                sender.mark_packet_sent(pkt);

                if (loss_emulator.should_drop()) {
                    dropped_count++;
                    std::cout << "[X DROP]   Packet " << pkt.header.sequence_number << " dropped.\n";
                } else {
                    auto serialized = pkt.serialize();
                    socket.send_to(serialized.data(), serialized.size(), receiver_ip, receiver_port);
                    std::cout << "[-> SENT]  Packet " << pkt.header.sequence_number << "\n";
                }
                sent_count++;
            }
        }

        ssize_t bytes = socket.receive_from(net_buffer, sizeof(net_buffer), from_ip, from_port);
        if (bytes > 0) {
            Packet incoming_pkt;
            if (Packet::deserialize(net_buffer, bytes, incoming_pkt)) {
                if (incoming_pkt.header.packet_type == static_cast<uint8_t>(PacketType::NACK)) {
                    uint32_t count = incoming_pkt.header.ack_or_nack_id;
                    const uint32_t* missing_seqs = reinterpret_cast<const uint32_t*>(incoming_pkt.payload);

                    // Unpack and retransmit all requested missing packets from the single NACK
                    for (uint32_t i = 0; i < count; ++i) {
                        uint32_t target_seq = missing_seqs[i];
                        Packet retransmit_pkt;
                        if (sender.get_packet_by_sequence(target_seq, retransmit_pkt)) {
                            auto serialized = retransmit_pkt.serialize();
                            socket.send_to(serialized.data(), serialized.size(), receiver_ip, receiver_port);
                            retransmitted_count++;
                            std::cout << "[⚡ RECOVERY] Handled batch NACK -> Retransmitted packet " << target_seq << "\n";
                        }
                    }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    std::cout << "\n[+] File transmission complete! Drops: " << dropped_count 
              << ", Recoveries: " << retransmitted_count << "\n";
    return 0;
}