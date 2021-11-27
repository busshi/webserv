#include "http/header.hpp"
#include "http/message.hpp"
#include <sstream>

HTTP::Header::Header(void)
{
}

HTTP::Header::Header(const HTTP::Header& other)
{
    *this = other;
}

HTTP::Header::~Header(void)
{
}

HTTP::Header& HTTP::Header::operator=(const Header &rhs)
{
    if (this != &rhs) {
        _fields = rhs._fields;
    }

    return *this;
}

std::string HTTP::Header::getField(const std::string& name) const
{
    Fields::const_iterator cit = _fields.find(name);

    return cit != _fields.end() ? cit->second : "";
}

HTTP::Header& HTTP::Header::setField(const std::string& name, const std::string& value)
{
    _fields[name] = value;

    return *this;
}

HTTP::Header& HTTP::Header::parse(const std::string &headerData)
{
    std::string::size_type bodyPos = headerData.find(HTTP::CRLF + HTTP::CRLF);

    std::vector<std::string> fields = split(headerData.substr(0, bodyPos), HTTP::CRLF);

    for (std::vector<std::string>::const_iterator cit = fields.begin(); cit != fields.end(); ++cit) {
        std::vector<std::string> ss = split(*cit, " ");
        setField(ss[0].substr(0, ss[0].find(':')), ss[1]);
    }

    return *this;
}

std::string HTTP::Header::format(void) const
{   
    std::ostringstream oss;
    for (Fields::const_iterator cit = _fields.begin(); cit != _fields.end(); ++cit) {
        oss << cit->first << ": " << cit->second << HTTP::CRLF;
    }

    return oss.str();
}