#include "../includes/server.hpp"

Server* Server::instance = NULL;

void Server::signalHandler(int)
{
    if (Server::instance)
    {
        write(Server::instance->shutdown_pipe[1], "x", 1);
    }
}

Server::Server(int port)
    : server_fd(-1), epoll_fd(-1), port(port), running(true)
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
void Server::setup()
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

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
        throw std::runtime_error("socket failed");

    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    int nb = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &nb, sizeof(nb)) < 0)
        throw std::runtime_error("setsockopt failed");    

    if (bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0)
        throw std::runtime_error("bind failed");

    if (listen(server_fd, SOMAXCONN) < 0)
        throw std::runtime_error("listen failed");

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

    std::cout << "Server running on port " << port << std::endl;
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

std::string handleCommands(char *buffer)
{
    std::string msg = "UNKNOWN COMMAND\r\n";
    if (strcmp(buffer, "PASS\n") == 0)
        msg = "handlePass();\n"; // handlePass();
    return msg;    
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

    if (bytes > 0)
    {
        buffer[bytes] = '\0';

        clients[fd].getReadBuffer().append(buffer, bytes);

        std::cout << "Client " << fd << ": "
              << buffer << std::endl;
    }

    std::string msg = handleCommands(buffer);

    send(fd, msg.c_str(), msg.size(), 0);
}

void Server::removeClient(int fd)
{
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
    clients.erase(fd);

    std::cout << "Client disconnected: " << fd << std::endl;
}