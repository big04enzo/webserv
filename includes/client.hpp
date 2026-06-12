#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
private:
    int fd;
    std::string nick_name;
    std::string user_name;
    std::string real_name;
    int pass_ok;
    int  registered;
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