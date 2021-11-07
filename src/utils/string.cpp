#include "utils/string.hpp"
#include <cctype>
#include <cstdlib>
#include <string>

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

static std::string
expandVariable(const std::string& path, size_t& pos)
{
    size_t beginPos = ++pos;

    while (pos < path.size() && isalnum(path[pos])) {
        ++pos;
    }

    const char* envVar = getenv(path.substr(beginPos, pos - beginPos).c_str());

    return envVar ? envVar : "";
}

std::string
expandVar(const std::string& path)
{
    std::string expanded;
    size_t pos = 0, fIndex = 0;

    while (pos < path.size() &&
           (fIndex = path.find('$', pos)) != std::string::npos) {
        expanded += path.substr(pos, fIndex - pos);
        /* The two add statements are separated because fIndex is mutated by
         * expandVariable: otherwise order of evaluation matters and is
         * unspecified */
        expanded += expandVariable(path, fIndex);
        pos = fIndex;
    }
    expanded += path.substr(pos, path.size() - pos);

    return expanded;
}
