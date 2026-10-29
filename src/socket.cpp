#include "socket.h"
#include <iostream>
#include <cstring>

UdpSocket::UdpSocket() : sockfd_(-1) {
    // Create a standard UDP socket (IPv4, Datagram)
    sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_ < 0) {
        std::perror("Failed to create socket");
    }
}

UdpSocket::~UdpSocket() {
    if (sockfd_ >= 0) {
        close(sockfd_);
    }
}

bool UdpSocket::bind(uint16_t port) {
    std::memset(&local_addr_, 0, sizeof(local_addr_));
    local_addr_.sin_family = AF_INET;
    local_addr_.sin_addr.s_addr = INADDR_ANY; // Listen on all network interfaces
    local_addr_.sin_port = htons(port);

    if (::bind(sockfd_, (struct sockaddr*)&local_addr_, sizeof(local_addr_)) < 0) {
        std::perror("Failed to bind socket");
        return false;
    }
    return true;
}

bool UdpSocket::set_non_blocking() {
    int flags = fcntl(sockfd_, F_GETFL, 0);
    if (flags == -1) {
        std::perror("fcntl F_GETFL failed");
        return false;
    }
    if (fcntl(sockfd_, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::perror("fcntl F_SETFL (O_NONBLOCK) failed");
        return false;
    }
    return true;
}

ssize_t UdpSocket::send_to(const uint8_t* data, size_t size, const std::string& dest_ip, uint16_t dest_port) {
    sockaddr_in dest_addr{};
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(dest_port);
    
    if (inet_pton(AF_INET, dest_ip.c_str(), &dest_addr.sin_addr) <= 0) {
        std::cerr << "Invalid destination IP address: " << dest_ip << "\n";
        return -1;
    }

    return sendto(sockfd_, data, size, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
}

ssize_t UdpSocket::receive_from(uint8_t* buffer, size_t max_size, std::string& sender_ip, uint16_t& sender_port) {
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);

    ssize_t bytes_received = recvfrom(sockfd_, buffer, max_size, 0, (struct sockaddr*)&client_addr, &client_len);
    
    if (bytes_received > 0) {
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
        sender_ip = ip_str;
        sender_port = ntohs(client_addr.sin_port);
    }

    return bytes_received;
}