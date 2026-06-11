#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
private:
    int fd;
    std::string readBuffer;
    std::string writeBuffer;

public:
    Client();
    Client(int fd);
    ~Client();

    int getFd() const;

    std::string& getReadBuffer();
    std::string& getWriteBuffer();
};

#endif