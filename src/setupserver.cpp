#include "../includes/server.hpp"

Server* Server::instance = NULL;

void Server::signalHandler(int)
{
    if (Server::instance)
    {
        write(Server::instance->shutdown_pipe[1], "x", 1);
    }
}

Server::Server(int port, std::string pass)
    : server_fd(-1), epoll_fd(-1), port(port), running(true), pass(pass)
{
    std::memset(&server_addr, 0, sizeof(server_addr));
    Server::instance = this;
}

Server::~Server()
{
    close(server_fd);
    close(epoll_fd);
    close(shutdown_pipe[0]);
    close(shutdown_pipe[1]);
}

void Server::setupSystem()
{
    if (pipe(shutdown_pipe) == -1)
        throw std::runtime_error("pipe failed");

    fcntl(shutdown_pipe[0], F_SETFL, O_NONBLOCK);
    fcntl(shutdown_pipe[1], F_SETFL, O_NONBLOCK);

    struct sigaction sa;
    std::memset(&sa, 0, sizeof(sa));

    sa.sa_handler = Server::signalHandler;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

void Server::setupSocket()
{
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
        throw std::runtime_error("socket failed");

    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    int nb = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &nb, sizeof(nb)) < 0)
        throw std::runtime_error("setsockopt failed");

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0)
        throw std::runtime_error("bind failed");

    if (listen(server_fd, SOMAXCONN) < 0)
        throw std::runtime_error("listen failed");
}

void Server::setupEpoll()
{
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
        throw std::runtime_error("epoll failed");

    epoll_event ev;

    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);

    ev.events = EPOLLIN;
    ev.data.fd = shutdown_pipe[0];
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, shutdown_pipe[0], &ev);
}


