#include "http/message.hpp"

std::string HTTP::Message::getHeaderField(const std::string& name) const
{
    std::map<std::string, std::string>::const_iterator cit =_headers.find(name);

    return cit != _headers.end() ? cit->second : "";
}

void HTTP::Message::setHeaderField(const std::string& name, const std::string& value)
{
    _headers[name] = value;
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