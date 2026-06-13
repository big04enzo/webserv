
#include "../includes/server.hpp"

void Server::acceptClient()
{
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0)
        return;

    fcntl(client_fd, F_SETFL, O_NONBLOCK);

    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = client_fd;

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);

    clients[client_fd] = Client(client_fd);

    std::cout << "Client connected: " << client_fd << std::endl;
}

void Server::readClient(int fd)
{
    char buffer[30000];

    int bytes = recv(fd, buffer, sizeof(buffer), 0);

    if (bytes <= 0)
    {
        removeClient(fd);
        return;
    }

    buffer[bytes] = '\0';

    clients[fd].getReadBuffer().append(buffer, bytes);

    std::string msg = handleCommands(buffer, fd);

    send(fd, msg.c_str(), msg.size(), 0);

    std::string std = commandcheck(fd);
    std::cout << std << std::endl;
}

void Server::removeClient(int fd)
{
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
    clients.erase(fd);

    std::cout << "Client disconnected: " << fd << std::endl;
}