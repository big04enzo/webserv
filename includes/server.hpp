#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <csignal>
#include <cstring>
#include <iostream>
#include <map>
#include <cstdlib>
#include <vector>
#include "client.hpp"

class Server
{
private:
    int server_fd;
    int epoll_fd;
    int port;
    bool running;
    std::string pass;

    int shutdown_pipe[2];

    sockaddr_in server_addr;

    std::map<int, Client> clients;

    struct epoll_event events[1024];

    static Server* instance;

    static void signalHandler(int sig);

    void acceptClient();
    void readClient(int fd);
    void removeClient(int fd);
    std::string handleCommands(char *buffer, int fd);
    std::string handlePass(std::vector<std::string> cmd, int fd);

public:
    Server(int port, std::string pass);
    ~Server();

    void setup();
    void run();
};

#endif