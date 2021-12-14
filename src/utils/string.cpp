#include "utils/string.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <sstream>
#include <string>

using std::reverse;
using std::string;
using std::vector;

/**
 * @brief Helper method for working with split. Returns the index where any of
 * the characters in set first occurs in s.
 *
 * @param s The string to search in
 * @param set A set of characters that are matchable
 * @param pos The position to start the search at in s
 * @return string::size_type the index where the first occurence of any
 * character in set is found, string::npos is there is not.
 */

static string::size_type
splitFindOneOfSet(const string& s, const string& set, string::size_type pos = 0)
{
    for (string::size_type i = pos; i != s.size(); ++i) {
        if (set.find(s[i]) != string::npos) {
            return i;
        }
    }

    return string::npos;
}

/**
 * @brief splits a string into one or several strings, each character of
 * set being a possible delimiter.
 *
 * @param s The string to split
 * @param set A string which each of its characters can be a delimiter to split
 * s
 * @return vector<string>  A vector of string that holds the split.
 */

vector<string>
split(const string& s, const string& set)
{
    vector<string> v;
    string::size_type bpos = 0, fpos = 0;

    while (bpos < s.size()) {
        while (set.find(s[bpos]) != string::npos) {
            ++bpos;
        }
        if (bpos == s.size()) {
            break;
        }
        fpos = splitFindOneOfSet(s, set, bpos);
        if (fpos == string::npos) {
            fpos = s.size();
        }
        v.push_back(s.substr(bpos, fpos - bpos));
        bpos = fpos;
    }

    return v;
}

string
trim(const string& s, const string& set)
{
    string::size_type bpos = 0, epos = s.size() - 1;

    while (set.find(s[bpos]) != string::npos)
        ++bpos;
    while (epos >= bpos && set.find(s[epos]) != string::npos)
        --epos;

    return s.substr(bpos, epos - bpos + 1);
}

string
trimTrailing(const string& s, const string& set)
{
    string::size_type epos = s.size() - 1;

    while (set.find(s[epos]) != string::npos) {
        if (epos == 0)
            return "";
        --epos;
    }

    return s.substr(0, epos + 1);
}

static string
expandVariable(const string& path, size_t& pos)
{
    size_t beginPos = ++pos;

    while (pos < path.size() && isalnum(path[pos])) {
        ++pos;
    }

    const char* envVar = getenv(path.substr(beginPos, pos - beginPos).c_str());

    return envVar ? envVar : "";
}

string
expandVar(const string& path)
{
    string expanded;
    size_t pos = 0, fIndex = 0;

    while (pos < path.size() &&
           (fIndex = path.find('$', pos)) != string::npos) {
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
isIPv4(const string& s)
{
    vector<string> ss = split(s, ".");
    std::istringstream iss;

    if (ss.size() != 4) {
        return false;
    }

    for (vector<string>::const_iterator cit = ss.begin(); cit != ss.end();
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

string
dirname(const string& s)
{
    string::size_type f = s.find_last_of('/');

    if (f != string::npos) {
        return s.substr(0, f);
    }

    return "";
}

string
toLowerCase(const string& s)
{
    string ls(s);

    for (string::iterator it = ls.begin(); it != ls.end(); ++it) {
        *it = tolower(*it);
    }

    return ls;
}

string
toUpperCase(const string& s)
{
    string us(s);

    for (string::iterator it = us.begin(); it != us.end(); ++it) {
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
parseInt(const string& s, int radix)
{
    static const string charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    long long parsed = 0;
    bool isNeg = false;
    string trimmed = trim(s);

    string::const_iterator sbit = trimmed.begin();

    while (sbit != trimmed.end() && (*sbit == '+' || *sbit == '-')) {
        if (*sbit == '-') {
            isNeg = !isNeg;
        }
        ++sbit;
    }

    while (sbit != trimmed.end()) {
        string::size_type pos = charset.find(toupper(*sbit));

        if (pos == string::npos) {
            break;
        }

        parsed = parsed * radix + pos;
        ++sbit;
    }

    return parsed * (isNeg ? -1 : 1);
}

string
ntos(long long n, int radix, bool lower)
{
    static const string charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    string s;
    bool isNeg = false;

    if (n < 0) {
        n = -n;
        isNeg = true;
    }

    while (n > 0) {
        s +=
          string(1, lower ? tolower(charset[n % radix]) : charset[n % radix]);
        n /= radix;
    }

    if (isNeg) {
        s += "-";
    }

    reverse(s.begin(), s.end());

    return s;
}

bool
hasFileExtension(const string& path, const string& fileExtension)
{
    return path.find(fileExtension) == path.size() - fileExtension.size();
}

bool
equalsIgnoreCase(const string& s1, const string& s2)
{
    if (s1.size() != s2.size()) {
        return false;
    }

    for (string::const_iterator first1 = s1.begin(), first2 = s2.begin();
         first1 != s1.end();
         ++first1, ++first2) {
        if (tolower(*first1) != tolower(*first2)) {
            return false;
        }
    }

    return true;
}

bool
isNumber(const string& s)
{
    string normalized = trim(s);
    string::size_type i = 0;

    if (normalized[i] == '+' || normalized[i] == '-') {
        ++i;
    }

    for (; i != normalized.size(); ++i) {
        if (!isdigit(normalized[i])) {
            return false;
        }
    }

    return true;
}
