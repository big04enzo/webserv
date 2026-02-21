#ifndef NETWORKSOCKET_HPP
#define NETWORKSOCKET_HPP

#include <string>

class NetworkSocket {
public:
    NetworkSocket(int port);
    void start();

private:
    int server_fd;
    int port;
    void setupSocket();
    std::string handleRequest(const std::string &request);
};

#endif