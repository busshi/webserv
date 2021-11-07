#include "utils/Formatter.hpp"
#include "utils/string.hpp"
#include <string>
#include <algorithm>

bool
validateAutoindex(const std::string& value, std::string& errorMsg)
{
    if (value == "off" || value == "on") {
        return true;
    }
    Formatter()
        << "autoindex only supports 'on' and 'off' as possible values, found \""
        << value << "\"" >>
      errorMsg;
    return false;
}

bool
validateMethod(const std::string& value, std::string& errorMsg)
{
    const char* availableMethods[] = {
        "GET",
        "POST",
        "PUT",
        "DELETE"
    };
    bool seen[sizeof(availableMethods) / sizeof(char*)] = { false };
    size_t n = sizeof(availableMethods) / sizeof(char*);

    std::vector<std::string> vs = split(value, ' ');

    for (std::vector<std::string>::const_iterator ite = vs.begin(); ite != vs.end(); ++ite) {
        size_t i = 0;
        while (i != n) {
            if (*ite == availableMethods[i]) {
                if (seen[i]) {
                    Formatter() << "Method " << *ite << " appears more than one time " >> errorMsg;
                    return false;
                }
                seen[i] = true;
                break ;
            }
            ++i;
        }
        if (i == n) {
            Formatter() << "Method " << *ite << " is not supported" >> errorMsg;
            return false;
        }
    }
    return true;
}

bool
validateIndex(const std::string& value, std::string& errorMsg)
{
    std::vector<std::string>  vs = split(value, ' ');
    std::vector<std::string> seen;

    for (std::vector<std::string>::const_iterator ite = vs.begin(); ite != vs.end(); ++ite) {
        if (std::find(seen.begin(), seen.end(), *ite) != seen.end()) {
            Formatter() << "Duplicated index file " << *ite >> errorMsg;
            return false;
        }
        seen.push_back(*ite);
    }

    return true;
}

bool
validateRoot(const std::string& value, std::string& errorMsg)
{
    if (value[0] != '/') {
        Formatter() << "Root must takes an absolute path as its argument" >> errorMsg;
        return false;
    }
    return true;
}
