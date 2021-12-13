#include "utils/string.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <sstream>
#include <string>

using std::reverse;

/**
 * @brief Helper method for working with split. Returns the index where any of
 * the characters in set first occurs in s.
 *
 * @param s The string to search in
 * @param set A set of characters that are matchable
 * @param pos The position to start the search at in s
 * @return std::string::size_type the index where the first occurence of any
 * character in set is found, std::string::npos is there is not.
 */

static std::string::size_type
splitFindOneOfSet(const std::string& s,
                  const std::string& set,
                  std::string::size_type pos = 0)
{
    for (std::string::size_type i = pos; i != s.size(); ++i) {
        if (set.find(s[i]) != std::string::npos) {
            return i;
        }
    }

    return std::string::npos;
}

/**
 * @brief splits a std::string into one or several strings, each character of
 * set being a possible delimiter.
 *
 * @param s The string to split
 * @param set A string which each of its characters can be a delimiter to split
 * s
 * @return std::vector<std::string>  A vector of string that holds the split.
 */

std::vector<std::string>
split(const std::string& s, const std::string& set)
{
    std::vector<std::string> v;
    std::string::size_type bpos = 0, fpos = 0;

    while (bpos < s.size()) {
        while (set.find(s[bpos]) != std::string::npos) {
            ++bpos;
        }
        if (bpos == s.size()) {
            break;
        }
        fpos = splitFindOneOfSet(s, set, bpos);
        if (fpos == std::string::npos) {
            fpos = s.size();
        }
        v.push_back(s.substr(bpos, fpos - bpos));
        bpos = fpos;
    }

    return v;
}

std::string
trim(const std::string& s, const std::string& set)
{
    std::string::size_type bpos = 0, epos = s.size() - 1;

    while (set.find(s[bpos]) != std::string::npos)
        ++bpos;
    while (epos >= bpos && set.find(s[epos]) != std::string::npos)
        --epos;

    return s.substr(bpos, epos - bpos + 1);
}

std::string
trimTrailing(const std::string& s, const std::string& set)
{
    std::string::size_type epos = s.size() - 1;

    while (set.find(s[epos]) != std::string::npos) {
        if (epos == 0)
            return "";
        --epos;
    }

    return s.substr(0, epos + 1);
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

bool
isIPv4(const std::string& s)
{
    std::vector<std::string> ss = split(s, ".");
    std::istringstream iss;

    if (ss.size() != 4) {
        return false;
    }

    for (std::vector<std::string>::const_iterator cit = ss.begin();
         cit != ss.end();
         ++cit) {
        int n = 0;

        iss.str(*cit);
        iss >> n;
        iss.clear();

        if (n > 255) {
            return false;
        }
    }

    return true;
}

std::string
dirname(const std::string& s)
{
    std::string::size_type f = s.find_last_of('/');

    if (f != std::string::npos) {
        return s.substr(0, f);
    }

    return "";
}

std::string
toLowerCase(const std::string& s)
{
    std::string ls(s);

    for (std::string::iterator it = ls.begin(); it != ls.end(); ++it) {
        *it = tolower(*it);
    }

    return ls;
}

std::string
toUpperCase(const std::string& s)
{
    std::string us(s);

    for (std::string::iterator it = us.begin(); it != us.end(); ++it) {
        *it = toupper(*it);
    }

    return us;
}

/**
 * @brief parse a string-encoded integer expressed in base radix, such as 2 <=
 * radix <= 36.
 *
 * @param s
 * @param radix
 * @return long long
 */

long long
parseInt(const std::string& s, int radix)
{
    static const std::string charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    long long parsed = 0;
    bool isNeg = false;
    std::string trimmed = trim(s);

    std::string::const_iterator sbit = trimmed.begin();

    while (sbit != trimmed.end() && (*sbit == '+' || *sbit == '-')) {
        if (*sbit == '-') {
            isNeg = !isNeg;
        }
        ++sbit;
    }

    while (sbit != trimmed.end()) {
        std::string::size_type pos = charset.find(toupper(*sbit));

        if (pos == std::string::npos) {
            break;
        }

        parsed = parsed * radix + pos;
        ++sbit;
    }

    return parsed * (isNeg ? -1 : 1);
}

std::string
ntos(long long n, int radix, bool lower)
{
    static const std::string charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string s;
    bool isNeg = false;

    if (n < 0) {
        n = -n;
        isNeg = true;
    }

    while (n > 0) {
        s += std::string(
          1, lower ? tolower(charset[n % radix]) : charset[n % radix]);
        n /= radix;
    }

    if (isNeg) {
        s += "-";
    }

    reverse(s.begin(), s.end());

    return s;
}

bool
hasFileExtension(const std::string& path, const std::string& fileExtension)
{
    return path.find(fileExtension) == path.size() - fileExtension.size();
}

bool
equalsIgnoreCase(const std::string& s1, const std::string& s2)
{
    if (s1.size() != s2.size()) {
        return false;
    }

    for (std::string::const_iterator first1 = s1.begin(), first2 = s2.begin();
         first1 != s1.end();
         ++first1, ++first2) {
        if (tolower(*first1) != tolower(*first2)) {
            return false;
        }
    }

    return true;
}
