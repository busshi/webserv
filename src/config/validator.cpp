#include "utils/Formatter.hpp"
#include "utils/string.hpp"
#include <algorithm>
#include <climits>
#include <string>
#include <sys/types.h>

/*
static bool
isValidAddressPort(const std::string& value, std::string& errorMsg)
{
    std::vector<std::string> vs = split(value, ":");

    if (vs.size() != 2) {
        Formatter() << "Failed to parse <address>:<port>" >> errorMsg;
        return false;
    }

    if (!isIPv4(vs[0])) {
        Formatter() << "\"" << vs[0] << "\"" << " is not a valid IPv4." >>
errorMsg; return false;
    }

    std::istringstream iss(vs[1]);
    unsigned short port = 0;

    if (!(iss >> port)) {
        Formatter() << "Could not parse port in <address>:<port>." >> errorMsg;
        return false;
    }

    return true;
 }
 */

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
    const char* availableMethods[] = { "GET", "POST", "PUT", "DELETE" };
    bool seen[sizeof(availableMethods) / sizeof(char*)] = { false };
    size_t n = sizeof(availableMethods) / sizeof(char*);

    std::vector<std::string> vs = split(value);

    for (std::vector<std::string>::const_iterator ite = vs.begin();
         ite != vs.end();
         ++ite) {
        size_t i = 0;
        while (i != n) {
            if (*ite == availableMethods[i]) {
                if (seen[i]) {
                    Formatter() << "Method " << *ite
                                << " appears more than one time " >>
                      errorMsg;
                    return false;
                }
                seen[i] = true;
                break;
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
    std::vector<std::string> vs = split(value);
    std::vector<std::string> seen;

    for (std::vector<std::string>::const_iterator ite = vs.begin();
         ite != vs.end();
         ++ite) {
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
        Formatter() << "Root must takes an absolute path as its argument" >>
          errorMsg;
        return false;
    }
    return true;
}

// listen [address:]port [default]
bool
validateListen(const std::string& value, std::string& errorMsg)
{
    size_t f = value.find(':');

    /* if there is an IPv4 part then ensure the IP is valid */
    if (f != std::string::npos && f != 0) {
        std::string sIPv4 = value.substr(0, f);
        if (!isIPv4(sIPv4)) {
            Formatter() << "\"" << sIPv4 << "\" is not a valid IPv4" >>
              errorMsg;
            return false;
        }
    }

    /* parse port: port validity is not checked at validation step, it is only
     * ensured that it fits in a 16-bit unsigned value */
    size_t portBegInd = f != std::string::npos ? f + 1 : 0;

    std::istringstream iss(value.substr(portBegInd, value.size() - portBegInd));
    unsigned long long port = 0;

    iss >> port;

    if (port > USHRT_MAX) {
        Formatter() << "\"" << port
                    << "\" is not a valid port number: ports must fit in an "
                       "unsigned 16-bit integer" >>
          errorMsg;
        return false;
    }

    std::string w;

    if (iss >> w && w != "default_server") {
        Formatter() << "Unexpected token \"" << w << "\" in listen directive" >>
          errorMsg;
        return false;
    }

    return true;
}

bool
validateSize(const std::string& value, std::string& errorMsg)
{
    const char units[] = { 'k', 'm' };
    std::istringstream iss(value);
    unsigned long long n;

    if (!(iss >> n)) {
        Formatter() << "Could not parse size \"" << value
                    << "\": invalid integer part" >>
          errorMsg;
    }

    std::string unit;
    iss >> unit;

    if (!unit.empty()) {
        for (unsigned i = 0; i != sizeof(units) / sizeof(*units); ++i) {
            for (unsigned i = 0; i != sizeof(units) / sizeof(*units); ++i) {
                if (tolower(unit[0]) == units[i] &&
                    ((unit.size() == 2 && tolower(unit[1]) == 'b') ||
                     unit.size() == 1)) {
                    return true;
                }
            }
        }
    }

    if (!unit.empty()) {
        Formatter() << "Invalid unit \"" << unit
                    << "\" Available units are: k[b],m[b]" >>
          errorMsg;
        return false;
    }

    return true;
}

bool
validateLogLevel(const std::string& value, std::string& errorMsg)
{
    std::string levelAsStr = toLowerCase(value);
    std::string levels[] = { "debug", "info", "warning", "error" };

    for (unsigned i = 0; i != sizeof(levels) / sizeof(*levels); ++i) {
        if (levelAsStr == levels[i]) {
            return true;
        }
    }

    Formatter() << "Unknown log level \"" << levelAsStr << "\"" >> errorMsg;

    return false;
}

bool
validateRedirect(const std::string& value, std::string& errorMsg)
{
    std::vector<std::string> vs = split(value);

    if (vs.size() != 1) {
        Formatter() << "Expected a single URL for HTTP redirection, but found "
                       "extra tokens!" >>
          errorMsg;
        return false;
    }

    return true;
}

// example: cgi_pass .php,.php2 php-cgi

bool
validateCgiPass(const std::string& value, std::string& errorMsg)
{
    std::vector<std::string> vs = split(value);

    if (vs.size() != 2) {
        Formatter() << "Malformed cgi_pass: expected format is: cgi_pass "
                       "file_extensions path/to/cgi-executable."
                    << "(hint: commas can be used to separate multiple file "
                       "extensions, but no space is allowed." >>
          errorMsg;
        return false;
    }

    return true;
}

bool
validateNumber(const std::string& value, std::string& errorMsg)
{
    std::vector<std::string> vs = split(value);

    if (vs.size() > 1) {
        Formatter() << "Found extra tokens while expecting a single number" >>
          errorMsg;
        return false;
    }

    if (!isNumber(vs[0])) {
        Formatter() << value << " is not a valid number" >> errorMsg;
        return false;
    }

    return true;
}

#include <iostream>

bool
validateErrorPage(const std::string& value, std::string& errorMsg)
{
    std::vector<std::string> vs = split(value);

    if (vs.size() >= 2) {
        for (size_t i = 0; i != vs.size() - 1; ++i) {
            if (!isNumber(vs[i])) {
                Formatter() << vs[i] << " is not a valid number" >> errorMsg;
                return false;
            }

            unsigned long long code = parseInt(vs[i], 10);

            if (!IS_HTTP_STATUS(code)) {
                Formatter() << vs[i] << " is not a valid HTTP status code" >>
                  errorMsg;
                return false;
            }

            if (!(code >= 400 && code <= 599)) {
                Formatter() << "Only 4xx and 5xx HTTP statuses can be matched "
                               "with a custom error page" >>
                  errorMsg;
                return false;
            }
        }
    }

    return true;
}

bool
validateBool(const std::string& value, std::string& errorMsg)
{
    if (!equalsIgnoreCase(value, "FALSE") && !equalsIgnoreCase(value, "TRUE")) {
        Formatter() << "Boolean directive only accepts 'true' and 'false' as "
                       "values (case insensitive)" >>
          errorMsg;
        return false;
    }

    return true;
}
