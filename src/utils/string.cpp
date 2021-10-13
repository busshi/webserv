#include "utils/string.hpp"
#include <cctype>

std::vector<std::string>
split(const std::string& s, unsigned char c)
{
    std::vector<std::string> v;
    std::string::size_type bpos = 0, fpos = 0;

    while (bpos < s.size()) {
        while (s[bpos] == c) {
            ++bpos;
        }
        if (bpos == s.size()) {
            break;
        }
        fpos = s.find(c, bpos);
        if (fpos == std::string::npos) {
            fpos = s.size();
        }
        v.push_back(s.substr(bpos, fpos - bpos));
        bpos = fpos;
    }

    return v;
}

std::string
trim(const std::string& s)
{
    std::string::size_type bpos = 0, epos = s.size() - 1;

    while (isspace(s[bpos]))
        ++bpos;
    while (epos >= bpos && isspace(s[epos]))
        --epos;

    return s.substr(bpos, epos - bpos + 1);
}
