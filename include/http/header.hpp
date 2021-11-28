#pragma once
#include "utils/string.hpp"
#include <map>
#include <ostream>
#include <string>

namespace HTTP {

/**
* @brief A header mainly being a collection of key-value pairs, this typedef
provides a cleaner and shorter way to refer to the underlying type that is a map
of string to string. Map ordering is case insensitive as header field names are.
*/

class Header
{
    struct compareIgnoreCase
    {
        bool operator()(const std::string& s1, const std::string& s2) const
        {
            const std::string s1Lower = toLowerCase(s1),
                              s2Lower = toLowerCase(s2);

            return std::lexicographical_compare(
              s1Lower.begin(), s1Lower.end(), s2Lower.begin(), s2Lower.end());
        }
    };

    typedef std::map<std::string, std::string, compareIgnoreCase> Fields;

    Fields _fields;

  public:
        Header(void);
        ~Header(void);
        Header(const Header& other);
        Header& operator=(const Header& rhs);

        std::string getField(const std::string& name) const;
        Header& setField(const std::string& name, const std::string& value);
        Header& parse(const std::string& rawData);
        Header& merge(const Header& other,
        std::pair<const std::string, std::string> (*transformer)(const std::pair<const std::string, std::string>& p) = 0);
        char** toEnv(void) const;

        std::string format(void) const;
};

}