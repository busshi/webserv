#include "utils/Formatter.hpp"
#include <string>

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