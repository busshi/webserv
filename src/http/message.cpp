#include "http/message.hpp"
#include "utils/string.hpp"
#include "logger/Logger.hpp"
#include <sstream>
#include <string>
#include <fstream>

/* HTTP::Message */

std::string HTTP::Message::getHeaderField(const std::string& name) const
{
    std::map<std::string, std::string>::const_iterator cit =_header.find(name);

    return cit != _header.end() ? cit->second : "";
}

void HTTP::Message::setHeaderField(const std::string& name, const std::string& value)
{
    _header[name] = value;
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
        _header = rhs._header;
    }

    return *this; 
}

/* HTTP::Request */

HTTP::Request::Request(std::string rawData)
{
    std::string::size_type pos = rawData.find(HTTP::CRLF);

    std::vector<std::string> ss = split(rawData.substr(0, pos));

    _method = ss[0];
    _resourceURI = ss[1];
    _protocol = ss[2];

    std::string::size_type bodyPos = rawData.find(HTTP::CRLF + HTTP::CRLF);

    std::vector<std::string> fields = split(rawData.substr(pos, bodyPos - pos), HTTP::CRLF);

    for (std::vector<std::string>::const_iterator cit = fields.begin(); cit != fields.end(); ++cit) {
        std::vector<std::string> ss = split(*cit, " ");
        setHeaderField(ss[0].substr(0, ss[0].find(':')), ss[1]);
    }

    _body = rawData.substr(bodyPos + 4, rawData.size() - bodyPos - 4);
    _URI = "http://" + getHeaderField("Host") + _resourceURI;
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

const std::string& HTTP::Request::getResourceURI(void) const
{
    return _resourceURI;
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
    for (std::map<std::string, std::string>::const_iterator cit = _header.begin(); cit != _header.end(); ++cit) {
        os << cit->first << " " << cit->second << "\n";
    }

    return os;
}

/* Response */

HTTP::Response::Response(const HTTP::Request& req): _statusCode(HTTP::OK), _req(req)
{
}

HTTP::Response::Response(const HTTP::Response& other): Message()
{
    *this = other;
}

HTTP::Response& HTTP::Response::operator=(const HTTP::Response& rhs)
{
    if (this != &rhs) {
        Message::operator=(rhs);
    }

    return *this;
}

HTTP::Response::~Response(void)
{
}

std::string HTTP::Response::str(void) const
{
    return _sendHeaders() + _body;
}

HTTP::Response& HTTP::Response::setStatus(StatusCode statusCode)
{
    _statusCode = statusCode;

    return *this;
}

HTTP::Response& HTTP::Response::setStatus(unsigned intStatusCode)
{
    _statusCode = static_cast<StatusCode>(intStatusCode);

    return *this;
}

const HTTP::Request& HTTP::Response::getReq(void) const
{
    return _req;
}

std::string HTTP::Response::_sendHeaders(void) const
{
    std::ostringstream oss;

    for (HTTP::Header::const_iterator cit = _header.begin(); cit != _header.end(); ++cit) {
        oss << cit->first << ": " << cit->second << HTTP::CRLF;
    }
    oss << HTTP::CRLF;

    return oss.str();
}

HTTP::Response& HTTP::Response::sendFile(const std::string& filepath)
{
    std::ifstream ifs(filepath.c_str());
    std::string fileContent;
    
    if (!ifs) {
        glogger << "Failed to open file " << filepath << " while building a response\n";
    } else {
        fileContent = std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
    }

    _body = fileContent;

    return *this;
}

HTTP::Response& HTTP::Response::send(const std::string& s)
{
    _body = s;
    
    return *this;
}

HTTP::Response& HTTP::Response::append(const std::string& s)
{
    _body += s;
    
    return *this;
}