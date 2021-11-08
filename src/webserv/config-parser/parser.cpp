#include "utils/string.hpp"
#include "webserv/config-parser/ConfigParser.hpp"
#include <sstream>

/*
 ** Parsing utilities to make our life easier.
 **
 ** Any use of these functions assume that the input string has been validated
 ** at some point, no error checking is done.
 */

ListenData
parseListen(const std::string& listenDirective)
{
    ListenData data = { .v4 = "0.0.0.0", .port = 0, .isDefault = false };

    std::string::size_type f = listenDirective.find(':');

    if (f != std::string::npos && f > 0) {
        data.v4 = listenDirective.substr(0, f);
    }

    std::istringstream iss(
      f != std::string ::npos
        ? listenDirective.substr(f + 1, listenDirective.size() - f - 1)
        : listenDirective);

    iss >> data.port;

    std::string w;

    iss >> w;
    data.isDefault = w == "default_server";

    return data;
}
