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

void Client::setPASS_ok(int value)
{
    pass_ok = value;
}

int Client::getPASS_ok() const
{
    return pass_ok;
}

void Client::setRegistered(int value)
{
    registered = value;
}

int Client::getRegistered() const
{
    return registered;
}

void Client::setNick(std::string nick)
{
    nick_name = nick;
}

std::string Client::getNick() const
{
    return nick_name;
}

void Client::setUsername(std::string user)
{
    user_name = user;
}

std::string Client::getUsername() const
{
    return user_name;
}

void Client::setRealname(std::string real)
{
    real_name = real;
}

std::string Client::getRealname() const
{
    return real_name;
}