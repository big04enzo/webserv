#include "../includes/client.hpp"

Client::Client() : fd(-1)
{}

Client::Client(int fd) : fd(fd)
{}

Client::~Client()
{}

int Client::getFd() const
{ 
    return fd;
}

std::string& Client::getReadBuffer()
{
    return readBuffer;
}
std::string& Client::getWriteBuffer()
{
    return writeBuffer;
}
