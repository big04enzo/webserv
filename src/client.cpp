#include "../includes/client.hpp"

Client::Client() : fd(-1), pass_ok(0), registered(0)
{}

Client::Client(int fd) : fd(fd), pass_ok(0), registered(0)
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
