#include "../includes/server.hpp"

std::vector<std::string> splitCommand(const char *buffer)
{
    std::vector<std::string> tokens;
    std::string line(buffer);

    while (!line.empty() &&
          (line[line.size() - 1] == '\n'
        || line[line.size() - 1] == '\r'))
        line.erase(line.size() - 1);

    size_t i = 0;

    while (i < line.size())
    {
        while (i < line.size() &&
              (line[i] == ' ' || line[i] == '\t'))
            i++;

        if (i >= line.size())
            break;

        size_t start = i;

        while (i < line.size() &&
              line[i] != ' ' &&
              line[i] != '\t')
            i++;

        tokens.push_back(line.substr(start, i - start));
    }

    return tokens;
}

std::string Server::handleCommands(const char *buffer, int fd)
{
    std::vector<std::string> cmd = splitCommand(buffer);

    if (cmd.empty())
        return "";
    if (cmd[0] == "PASS")
        return handlePass(cmd, fd);
    if (cmd[0] == "NICK")
        return handleNick(cmd,fd);
    if (cmd[0] == "USER")
        return handleUser(cmd, fd);
    return ":IRC.SERV 421 " + cmd[0] + " :Unknown command\r\n";
}

void Server::logState(int fd, const std::string &msg)
{
    std::cout << "[CLIENT " << fd << "] " << msg << std::endl;
}