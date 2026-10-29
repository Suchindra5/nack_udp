#ifndef SOCKET_H
#define SOCKET_H

#include <string>
#include <vector>
#include <cstdint>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

class UdpSocket {
private:
    int sockfd_;
    sockaddr_in local_addr_;

public:
    UdpSocket();
    ~UdpSocket();

    // Bind socket to a local port (for listening or sending from a specific port)
    bool bind(uint16_t port);

    // Set socket to non-blocking mode (essential for busy-polling)
    bool set_non_blocking();

    // Send data to a specific destination IP and port
    ssize_t send_to(const uint8_t* data, size_t size, const std::string& dest_ip, uint16_t dest_port);

    // Receive data from any sender (populates sender_ip and sender_port)
    ssize_t receive_from(uint8_t* buffer, size_t max_size, std::string& sender_ip, uint16_t& sender_port);

    // Get the underlying file descriptor if needed
    int get_descriptor() const { return sockfd_; }
};

#endif // SOCKET_H