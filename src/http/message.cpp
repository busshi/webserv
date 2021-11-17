#include "http/message.hpp"
#include "utils/string.hpp"

/* HTTP::Message */

std::string HTTP::Message::getHeaderField(const std::string& name) const
{
    std::map<std::string, std::string>::const_iterator cit =_headers.find(name);

    return cit != _headers.end() ? cit->second : "";
}

void HTTP::Message::setHeaderField(const std::string& name, const std::string& value)
{
    _headers[name] = value;
}

HTTP::Message::Message(void)
{
}

HTTP::Message::~Message(void)
{
}

HTTP::Message::Message(const Message& other)
{
    *this = other;
}

HTTP::Message& HTTP::Message::operator=(const Message& rhs)
{
    if (this != &rhs) {
        _headers = rhs._headers;
    }

    return *this; 
}

/* HTTP::Request */

HTTP::Request::Request(std::string rawData)
{
    std::string::size_type pos = rawData.find(HTTP::CRLF);

    std::vector<std::string> ss = split(rawData.substr(0, pos));

    _method = ss[0];
    _URI = ss[1];
    _protocol = ss[2];

    std::string::size_type bodyPos = rawData.find(HTTP::CRLF + HTTP::CRLF);

    std::vector<std::string> fields = split(rawData.substr(pos, bodyPos - pos), HTTP::CRLF);

    for (std::vector<std::string>::const_iterator cit = fields.begin(); cit != fields.end(); ++cit) {
        std::vector<std::string> ss = split(*cit, " ");
        setHeaderField(ss[0].substr(0, ss[0].find(':')), ss[1]);
    }

    _body = rawData.substr(bodyPos + 4, rawData.size() - bodyPos - 4);
}

HTTP::Request::~Request(void)
{
}

HTTP::Request::Request(const HTTP::Request& other): Message()
{
    *this = other;
}

HTTP::Request& HTTP::Request::operator=(const HTTP::Request& rhs)
{
    Message::operator=(rhs);
    return *this;
}

const std::string& HTTP::Request::getMethod(void) const
{
    return _method;
}

const std::string& HTTP::Request::getURI(void) const
{
    return _URI;
}

const std::string& HTTP::Request::getProtocol(void) const
{
    return _protocol;
}

const std::string& HTTP::Request::getBody(void) const
{
    return _body;
}

std::ostream& HTTP::Request::printHeaders(std::ostream& os)
{
    for (std::map<std::string, std::string>::const_iterator cit = _headers.begin(); cit != _headers.end(); ++cit) {
        os << cit->first << " " << cit->second << "\n";
    }

    return os;
}