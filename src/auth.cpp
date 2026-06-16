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

std::string Server::generateWelcome(const std::string &nick)
{
    std::time_t now = std::time(NULL);
    std::tm *lt = std::localtime(&now);

    char timeBuf[64];
    std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", lt);

    std::string welcome;

    welcome += ":IRC.SERV 001 " + nick +
               " :Welcome to the IRC network\r\n";
    welcome += ":IRC.SERV 002 " + nick +
               " :Your host is IRC.SERV, running version 1.0\r\n";

    welcome += ":IRC.SERV 003 " + nick +
               " :This server was created " + std::string(timeBuf) + "\r\n";

    welcome += ":IRC.SERV 004 " + nick +
               " IRC.SERV 1.0 itkol\r\n";

    return welcome;
}

std::string Server::handleUser(std::vector<std::string> cmd, int fd)
{
    if (clients[fd].getPASS_ok() != 1)
        return ":IRC.SERV 464 USER :Password required\r\n";

    if (cmd.size() < 5)
        return ":IRC.SERV 461 USER :Not enough parameters\r\n";

    if (clients[fd].getRegistered())
        return ":IRC.SERV 462 USER :You may not reregister\r\n";

    std::string realname = reconstructRealName(cmd);

    if (!isValid(cmd[1]) || !isValidRealName(realname))
        return ":IRC.SERV 461 USER :Bad format\r\n";

    if (cmd[2] != "0" || cmd[3] != "*")
        return ":IRC.SERV 461 USER :Invalid parameters\r\n";

    clients[fd].setUsername(cmd[1]);
    clients[fd].setRealname(realname);
    logState(fd, "User set");
    if (!clients[fd].getNick().empty())
    {
        clients[fd].setRegistered(1);
        logState(fd, "Client fully registered");
        return generateWelcome(clients[fd].getNick() );
    }
    return "\r\n";
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

    std::string old_nick = clients[fd].getNick();

    clients[fd].setNick(cmd[1]);

    if (!old_nick.empty() && clients[fd].getRegistered())
    {
        logState(fd, "Nick changed from " + old_nick + " to " + clients[fd].getNick());
        return ":" + old_nick + "!" +
               clients[fd].getUsername() +
               "@localhost NICK " +
               cmd[1] + "\r\n";
    }

    if (!clients[fd].getRealname().empty())
    {
        clients[fd].setRegistered(1);
        logState(fd, "Client fully registered");
        return generateWelcome(cmd[1]);
    }
    logState(fd, "Nick set to " + clients[fd].getNick());
    return "\r\n";
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
        logState(fd, "Password accepted");
        return "\r\n";
    }
    return ":IRC.SERV 464 PASS :Password incorrect\r\n";
}