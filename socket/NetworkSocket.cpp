#include "NetworkSocket.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <vector>
#include <poll.h>
#include <sstream>
#include <fstream>

NetworkSocket::NetworkSocket(int port) : port(port), server_fd(-1) {}

void NetworkSocket::setupSocket() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    fcntl(server_fd, F_SETFL, O_NONBLOCK);
}

void NetworkSocket::start() {
    setupSocket();

    std::vector<pollfd> fds;
    fds.push_back({server_fd, POLLIN, 0});

    while (true) {
        int poll_count = poll(fds.data(), fds.size(), -1);
        if (poll_count < 0) {
            perror("Poll failed");
            break;
        }

        for (size_t i = 0; i < fds.size(); ++i) {
            if (fds[i].revents & POLLIN) {
                if (fds[i].fd == server_fd) {
                    sockaddr_in client_address;
                    socklen_t client_len = sizeof(client_address);
                    int client_socket = accept(server_fd, (struct sockaddr*)&client_address, &client_len);
                    if (client_socket >= 0) {
                        fcntl(client_socket, F_SETFL, O_NONBLOCK);
                        fds.push_back({client_socket, POLLIN, 0});
                    }
                } else {
                    char buffer[1024] = {0};
                    int bytes_read = read(fds[i].fd, buffer, sizeof(buffer));
                    if (bytes_read > 0) {
                        std::string response = handleRequest(buffer);
                        send(fds[i].fd, response.c_str(), response.size(), 0);
                    } else {
                        close(fds[i].fd);
                        fds.erase(fds.begin() + i);
                        --i;
                    }
                }
            }
        }
    }

    close(server_fd);
}
//hadi dialk al khraoui ra ghire vibe coditha
std::string NetworkSocket::handleRequest(const std::string &request) {
    std::istringstream request_stream(request);
    std::string method, path, version;
    request_stream >> method >> path >> version;

    if (method == "GET") {
        std::ifstream file("." + path);
        if (file) {
            std::ostringstream response;
            response << "HTTP/1.1 200 OK\r\n";
            response << "Content-Type: text/plain\r\n\r\n";
            response << file.rdbuf();
            return response.str();
        } else {
            return "HTTP/1.1 404 Not Found\r\n\r\n";
        }
    }

    return "HTTP/1.1 400 Bad Request\r\n\r\n";
}