#include "utils/Formatter.hpp"
#include <sstream>

std::ostringstream Formatter::baseStream;

Formatter
operator>>(Formatter formatter, std::string& s)
{
    s = formatter.baseStream.str();
    formatter.baseStream.str();

    return formatter;
}