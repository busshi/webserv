#include "http/header.hpp"
#include "http/message.hpp"
#include <cstring>
#include <sstream>

HTTP::Header::Header(void) {}

HTTP::Header::Header(const HTTP::Header& other)
{
    *this = other;
}

HTTP::Header::~Header(void) {}

HTTP::Header&
HTTP::Header::operator=(const Header& rhs)
{
    if (this != &rhs) {
        _fields = rhs._fields;
    }

    return *this;
}

std::string
HTTP::Header::getField(const std::string& name) const
{
    Fields::const_iterator cit = _fields.find(name);

    return cit != _fields.end() ? cit->second : "";
}

HTTP::Header&
HTTP::Header::setField(const std::string& name, const std::string& value)
{
    _fields[name] = value;

    return *this;
}

HTTP::Header&
HTTP::Header::parse(const std::string& headerData)
{
    std::string::size_type bodyPos = headerData.find(HTTP::BODY_DELIMITER);

    std::vector<std::string> fields =
      split(headerData.substr(0, bodyPos), CRLF);

    for (std::vector<std::string>::const_iterator cit = fields.begin();
         cit != fields.end();
         ++cit) {
        std::vector<std::string> ss = split(*cit, " ");
        setField(ss[0].substr(0, ss[0].find(':')), ss[1]);
    }

    return *this;
}

/**
 * @brief Merge all the header fields held by other in the current instance. In
 * case a header is already set before merging, the old value is replaced by the
 * new one.
 *
 * @param other
 * @return HTTP::Header&
 */

HTTP::Header&
HTTP::Header::merge(const HTTP::Header& other,
                    std::pair<const std::string, std::string> (*transformer)(
                      const std::pair<const std::string, std::string>& p))
{
    for (Fields::const_iterator cit = other._fields.begin();
         cit != other._fields.end();
         ++cit) {
        std::pair<const std::string, std::string> p =
          transformer ? transformer(*cit) : *cit;

        _fields[p.first] = p.second;
    }

    return *this;
}

char**
HTTP::Header::toEnv(void) const
{
    char **envp = new char *[_fields.size() + 1], **p = envp;

    for (Fields::const_iterator cit = _fields.begin(); cit != _fields.end();
         ++cit) {
        *p = strdup((cit->first + "=" + cit->second).c_str());
        ++p;
    }

    envp[_fields.size()] = 0;

    return envp;
}

std::string
HTTP::Header::format(void) const
{
    std::ostringstream oss;
    for (Fields::const_iterator cit = _fields.begin(); cit != _fields.end();
         ++cit) {
        oss << cit->first << ": " << cit->second << CRLF;
    }

    return oss.str();
}

HTTP::Header&
HTTP::Header::clear(void)
{
    _fields.clear();

    return *this;
}
