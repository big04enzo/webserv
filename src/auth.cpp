#include "../includes/server.hpp"

bool isValid(const std::string &nick)
{
    if (nick.empty())
        return false;

    for (size_t i = 0; i < nick.size(); i++)
    {
        char c = nick[i];
        if (c <= 31 || c == 127)
            return false;

        if (c == ' ' || c == '\t')
            return false;

        if (!( (c >= 'a' && c <= 'z') ||
               (c >= 'A' && c <= 'Z') ||
               (c >= '0' && c <= '9') ||
               c == '-' || c == '_' ||
               c == '[' || c == ']' ||
               c == '{' || c == '}' ||
               c == '|' ))
        {
            return false;
        }
    }
    return true;
}

bool Server::isNickUnique(const std::string &nick, int fd)
{
    std::map<int, Client>::iterator it;

    for (it = clients.begin(); it != clients.end(); ++it)
    {
        if (it->first == fd)
            continue;
        if (it->second.getNick() == nick)
            return false;
    }
    return true;
}

bool isValidRealName(const std::string &realname)
{
    if (realname.empty())
        return false;

    for (size_t i = 0; i < realname.size(); i++)
    {
        char c = realname[i];

        if (c <= 31 || c == 127)
            return false;
    }

    return true;
}

std::string reconstructRealName(const std::vector<std::string> &cmd)
{
    std::string realname;

    if (cmd.size() < 5)
        return "";

    if (cmd[4].empty() || cmd[4][0] != ':')
        return "";

    realname += cmd[4].substr(1);

    for (size_t i = 5; i < cmd.size(); i++)
    {
        realname += " ";
        realname += cmd[i];
    }

    return realname;
}

std::string Server::handleUser(std::vector<std::string> cmd, int fd)
{
    if (clients[fd].getPASS_ok() != 1)
        return ":IRC.SERV 464 USER :Password required\r\n";
    if (cmd.size() < 5)
        return ":IRC.SERV 461 USER :Not enough parameters\r\n";
    if (clients[fd].getRegistered() || !clients[fd].getRealname().empty())
        return ":IRC.SERV 462 USER :You may not reregister\r\n";
    std::string realname = reconstructRealName(cmd);
    if (!isValid(cmd[1]) || !isValidRealName(realname))
        return ":IRC.SERV 461 USER :Bad format\r\n";
    if (cmd[2] != "0" || cmd[3] != "*")
        return ":IRC.SERV 461 USER :Invalid parameters\r\n";
    clients[fd].setUsername(cmd[1]);
    clients[fd].setRealname(realname);
    if (!clients[fd].getNick().empty())
    {
        clients[fd].setRegistered(1);
        return ":IRC.SERV 001 :Welcome to the IRC network\r\n";
    }
    return "";
}

std::string Server::handleNick(std::vector<std::string> cmd, int fd)
{
    if (clients[fd].getPASS_ok() != 1)
        return ":IRC.SERV 464 NICK :Password required\r\n";
    if (cmd.size() != 2)
        return ":IRC.SERV 461 NICK :Invalid parameters\r\n";
    if (!isValid(cmd[1]))
        return ":IRC.SERV 432 NICK :Erroneous nickname\r\n";
    if (!isNickUnique(cmd[1], fd))
        return ":IRC.SERV 433 * " + cmd[1] + " :Nickname is already in use\r\n";
    clients[fd].setNick(cmd[1]);
    if (!clients[fd].getRealname().empty())
    {
        clients[fd].setRegistered(1);
        return ":IRC.SERV 001 :Welcome to the IRC network\r\n";
    }
    return "";
}

std::string Server::handlePass(std::vector<std::string> cmd, int fd)
{
    if (clients[fd].getPASS_ok() == 1)
        return ":IRC.SERV 462 PASS :Unauthorized command\r\n";

    if (cmd.size() != 2)
        return ":IRC.SERV 461 PASS :Invalid parameters\r\n";

    if (cmd[1] == pass)
    {
        clients[fd].setPASS_ok(1);
        return "";
    }
    return ":IRC.SERV 464 PASS :Password incorrect\r\n";
}