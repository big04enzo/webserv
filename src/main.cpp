#include "../includes/server.hpp"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }

    int port = std::atoi(argv[1]);

    if (port <= 0 || port > 65535)
    {
        std::cerr << "Invalid port" << std::endl;
        return 1;
    }

    std::string password = argv[2];

    if (password.empty())
    {
        std::cerr << "Password cannot be empty" << std::endl;
        return 1;
    }
    try
    {
        Server server(port, password);

        server.setup();
        server.run();

    }
    catch (const std::exception &e)
    {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Server exited cleanly" << std::endl;
    return 0;
}