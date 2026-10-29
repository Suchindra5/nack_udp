#include "protocol.h"
#include "sender.h"
#include "receiver.h"
#include <iostream>
#include <cassert>
#include <cstring>

void test_serialization() {
    std::cout << "[Test] Running packet serialization & deserialization check...\n";
    
    Packet original{};
    original.header.packet_type = static_cast<uint8_t>(PacketType::DATA);
    original.header.sequence_number = 42;
    original.header.ack_or_nack_id = 0;
    original.header.payload_length = 13;
    std::memcpy(original.payload, "Hello, World!", 13);

    auto serialized = original.serialize();
    
    Packet deserialized{};
    bool success = Packet::deserialize(serialized.data(), serialized.size(), deserialized);

    assert(success == true);
    assert(deserialized.header.packet_type == original.header.packet_type);
    assert(deserialized.header.sequence_number == 42);
    assert(deserialized.header.payload_length == 13);
    assert(std::memcmp(deserialized.payload, "Hello, World!", 13) == 0);

    std::cout << " -> Serialization test passed successfully!\n";
}

void test_receiver_gap_detection() {
    std::cout << "[Test] Running receiver gap-detection & NACK generation check...\n";
    
    Receiver receiver;

    // Packet 0 arrives smoothly
    Packet p0{};
    p0.header.packet_type = static_cast<uint8_t>(PacketType::DATA);
    p0.header.sequence_number = 0;
    receiver.process_packet(p0);
    assert(!receiver.has_pending_nacks());

    // Packet 2 arrives out-of-order, creating a gap (missing Packet 1)
    Packet p2{};
    p2.header.packet_type = static_cast<uint8_t>(PacketType::DATA);
    p2.header.sequence_number = 2;
    receiver.process_packet(p2);

    // Verify a NACK was fired for sequence 1
    assert(receiver.has_pending_nacks());
    auto nacks = receiver.get_and_clear_nacks();
    assert(nacks.size() == 1);
    assert(nacks[0].header.packet_type == static_cast<uint8_t>(PacketType::NACK));
    assert(nacks[0].header.ack_or_nack_id == 1); // Pinpointed missing packet ID

    std::cout << " -> Receiver gap detection test passed successfully!\n";
}

void test_sender_window() {
    std::cout << "[Test] Running sender sliding window capacity check...\n";
    
    Sender sender(4); // Small window size of 4
    assert(sender.can_send() == true);

    Packet p;
    uint8_t data[] = "test";
    
    // Fill the window completely
    for (int i = 0; i < 4; ++i) {
        assert(sender.can_send() == true);
        sender.create_data_packet(data, 4, p);
        sender.mark_packet_sent(p);
    }

    // Window should now be blocked
    assert(sender.can_send() == false);

    std::cout << " -> Sender window test passed successfully!\n";
}

int main() {
    std::cout << "\n==================================================\n";
    std::cout << "      RUNNING PROTOCOL UNIT TEST SUITE            \n";
    std::cout << "==================================================\n";
    
    test_serialization();
    test_receiver_gap_detection();
    test_sender_window();

    std::cout << "\n==================================================\n";
    std::cout << "   ALL UNIT TESTS PASSED WITH 0 ERRORS!           \n";
    std::cout << "==================================================\n\n";
    return 0;
}