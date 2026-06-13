#include "../includes/server.hpp"

void Server::setup()
{
    setupSystem();
    setupSocket();
    setupEpoll();

    std::cout << ":IRC.SERV 001 :Welcome! Please register" << std::endl;
    std::cout << ":IRC.SERV 001 :PASS <server-password>" << std::endl;
    std::cout << ":IRC.SERV 001 :NICK <nickname>" << std::endl;
    std::cout << ":IRC.SERV 001 :USER <user> 0 * :<real name>" << std::endl;
}

void Server::run()
{
    while (running)
    {
        int n = epoll_wait(epoll_fd, events, 1024, -1);

        if (n <= 0)
            continue;

        for (int i = 0; i < n; i++)
        {
            int fd = events[i].data.fd;
            if (fd == shutdown_pipe[0])
            {
                char buf[64];
                read(fd, buf, sizeof(buf));
                running = false;
                break;
            }
            if (fd == server_fd)
            {
                acceptClient();
                continue;
            }

            if (events[i].events & EPOLLIN)
                readClient(fd);
        }
    }

    std::cout << "Shutting down server..." << std::endl;
}