#pragma once
#include <sstream>

class Formatter
{
  public:
    static std::ostringstream baseStream;
};

Formatter
operator>>(Formatter formatter, std::string& s);

template<typename T>
Formatter
operator<<(Formatter formatter, const T& printable)
{
    formatter.baseStream << printable;
    return formatter;
}
