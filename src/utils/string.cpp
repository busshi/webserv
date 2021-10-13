#include "utils/string.hpp"

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