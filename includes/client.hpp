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

public:
    Client();
    Client(int fd);
    ~Client();

    int getFd() const;
    void setPASS_ok(int) ;
    int getRegistered() const;
    void setRegistered(int) ;
    int getPASS_ok() const;
    void setNick(std::string nick) ;
    std::string getNick() const;
    void setUsername(std::string nick) ;
    std::string getUsername() const;
    void setRealname(std::string nick) ;
    std::string getRealname() const;
    std::string& getReadBuffer();
};

#endif