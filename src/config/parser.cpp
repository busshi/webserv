#include "config/ConfigItem.hpp"
#include "config/ConfigParser.hpp"
#include "utils/string.hpp"
#include <arpa/inet.h>
#include <cmath>
#include <netinet/in.h>
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
    ListenData data = { .v4 = INADDR_ANY, .port = 0, .isDefault = false };

    std::string::size_type f = listenDirective.find(':');

    if (f != std::string::npos && f > 0) {
        data.v4 = inet_addr(listenDirective.substr(0, f).c_str());
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

/*
 * Returns the number of byte corresponding to the passed size expression.
 * For example, "100kb" will return "100000".
 */

unsigned long long
parseSize(const std::string& size)
{
    const char units[] = { 'k', 'm' };
    std::istringstream iss(size);
    unsigned long long n = 0;
    std::string unit;

    iss >> n >> unit;

    for (unsigned i = 0; i != sizeof(units) / sizeof(*units); ++i) {
        if (tolower(unit[0]) == units[i] &&
            ((unit.size() == 2 && tolower(unit[1]) == 'b') ||
             unit.size() == 1)) {
            return n * (1 << (10 * (i + 1)));
        }
    }

    return n;
}

/**
 * first: a vector that holds all the status code that are matching a given
 * error page second: path to the error page
 */

std::map<unsigned int, std::string>
parseErrorPage(ConfigItem* current)
{
    std::map<unsigned int, std::string> m;
    std::vector<ConfigItem*> errorDirs =
      current->findNearestBlocks("error_page");

    for (size_t i = 0; i != errorDirs.size(); ++i) {
        std::vector<std::string> vs = split(errorDirs[i]->getValue());

        for (size_t j = 0; j != vs.size() - 1; ++j) {
            unsigned int code = parseInt(vs[j], 10);

            if (m.find(code) == m.end()) {
                m[code] = vs.back();
            }
        }
    }

    return m;
}
