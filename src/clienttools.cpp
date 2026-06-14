#include "../includes/server.hpp"

bool safeRecv(int fd, std::string &out)
{
    char buffer[4096];

    int bytes = recv(fd, buffer, sizeof(buffer), 0);

    if (bytes == 0)
        return false;

    if (bytes < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return true;
        return false;
    }

    buffer[bytes] = '\0';
    out.append(buffer, bytes);

    return true;
}

bool safeSend(int fd, const std::string &msg)
{
    size_t totalSent = 0;

    while (totalSent < msg.size())
    {
        int sent = send(fd,
                        msg.c_str() + totalSent,
                        msg.size() - totalSent,
                        0);

        if (sent <= 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                continue;

            return false;
        }
        totalSent += sent;
    }

    return true;
}

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

    std::cout << "[CLIENT " << client_fd << "] " << " connected "<< std::endl;
    std::cout << ":IRC.SERV 001 :Welcome! Please register" << std::endl;
    std::cout << ":IRC.SERV 001 :PASS <server-password>" << std::endl;
    std::cout << ":IRC.SERV 001 :NICK <nickname>" << std::endl;
    std::cout << ":IRC.SERV 001 :USER <user> 0 * :<real name>" << std::endl;
}

void Server::readClient(int fd)
{
    std::string data;

    if (!safeRecv(fd, data))
    {
        removeClient(fd);
        return ;
    }

    clients[fd].getReadBuffer().append(data);

    std::string msg = handleCommands(data.c_str(), fd);

    if (!msg.empty())
    {
        if (!safeSend(fd, msg))
        {
            removeClient(fd);
            return ;
        }
    }
}

void Server::removeClient(int fd)
{
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
    clients.erase(fd);
    std::cout << "[CLIENT " << fd << "] " << "disconnected" << std::endl;
}