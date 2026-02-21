#include "socket/NetworkSocket.hpp"

int main(int ac ,char **av) {
    NetworkSocket server(8080);
    server.start();
    return 0;
}