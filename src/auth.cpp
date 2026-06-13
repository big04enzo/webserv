#include "../includes/server.hpp"

bool isValidNick(const std::string &nick)
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

std::string Server::handleNick(std::vector<std::string> cmd, int fd)
{
    if (clients[fd].getPASS_ok() != 1)
        return ":IRC.SERV 464 NICK :Password required\r\n";
    if (cmd.size() < 2)
        return ":IRC.SERV 461 NICK :Not enough parameters\r\n";
    if (!isValidNick(cmd[1]))
        return ":IRC.SERV 432 NICK :Erroneous nickname\r\n";
    if (!isNickUnique(cmd[1], fd))
        return ":IRC.SERV 433 * " + cmd[1] + " :Nickname is already in use\r\n";
    else
    {
        clients[fd].setNick(cmd[1]);
        return "";
    }
}

std::string Server::handlePass(std::vector<std::string> cmd, int fd)
{
    if (clients[fd].getPASS_ok() == 1)
        return ":IRC.SERV 462 PASS ::Unauthorized command\r\n";

    if (cmd.size() < 2)
        return ":IRC.SERV 461 PASS :Not enough parameters\r\n";

    if (cmd[1] == pass)
    {
        clients[fd].setPASS_ok(1);
        return "";
    }
    return ":IRC.SERV 464 PASS :Password incorrect\r\n";
}