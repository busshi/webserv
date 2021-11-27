#include "http/message.hpp"
#include "utils/string.hpp"
#include "logger/Logger.hpp"
#include <sstream>
#include <string>
#include <fstream>

/* HTTP::Message */

/**
 * @brief retrieve the value of a given header field.
 * 
 * @param name The name of the field, which is case *insensitive*
 * @return std::string 
 */

std::string HTTP::Message::getHeaderField(const std::string& name) const
{
    return _header.getField(name);
}

/**
 * @brief Set the value of a given header field.
 * 
 * @param name The name of the field, which is case *insensitive*
 * @param value The value to assign to the field. If the field already has a value, it gets replaced by the new one.
 */

void HTTP::Message::setHeaderField(const std::string& name, const std::string& value)
{
    _header.setField(name, value);
}

/**
 * @brief Construct a new HTTP::Message::Message object
 * 
 */

HTTP::Message::Message(void)
{
}

/**
 * @brief Destroy the HTTP::Message::Message object
 * 
 */

HTTP::Message::~Message(void)
{
}

/**
 * @brief Construct a new HTTP::Message::Message object
 * 
 * @param other
 */

HTTP::Message::Message(const Message& other)
{
    *this = other;
}

/**
 * @brief Copy data of rhs into lhs
 * 
 * @param rhs 
 * @return HTTP::Message& 
 */

HTTP::Message& HTTP::Message::operator=(const Message& rhs)
{
    if (this != &rhs) {
        _header = rhs._header;
    }

    return *this; 
}

HTTP::Request::Request(void)
{
}

/**
 * @brief Construct a new HTTP::Request::Request object
 * 
 * @param rawData Basically a stream of bytes, here passed as a std::string. 
 It is usually the data sent by the user agent through the client socket.
 */

HTTP::Request::Request(int csockFd, const std::string& headerRawData): _csockFd(csockFd)
{
    _parseHeader(headerRawData);
}

 HTTP::Request& HTTP::Request::_parseHeader(const std::string& headerData)
 {
   std::string::size_type pos = headerData.find(HTTP::CRLF);

    std::vector<std::string> ss = split(headerData.substr(0, pos));

    _method = ss[0];
    _resourceURI = ss[1];
    _protocol = ss[2];

    _header.parse(headerData.substr(pos, std::string::npos));

    _URI = "http://" + getHeaderField("Host") + _resourceURI;

    return *this;
 }

/**
 * @brief Destroy the HTTP::Request::Request object
 * 
 */

HTTP::Request::~Request(void)
{
}

/**
 * @brief Construct a new HTTP::Request::Request object
 * 
 * @param other 
 */

HTTP::Request::Request(const HTTP::Request& other): Message()
{
    *this = other;
}

/**
 * @brief Copy data of rhs to lhs
 * 
 * @param rhs 
 * @return HTTP::Request& 
 */

HTTP::Request& HTTP::Request::operator=(const HTTP::Request& rhs)
{
    Message::operator=(rhs);
    if (this != &rhs) {
        _method = rhs._method;
        _resourceURI = rhs._resourceURI;
        _URI = rhs._URI;
        _protocol = rhs._protocol;
        body.str("");
        body << rhs.body.str();
    }
    return *this;
}

/**
 * @brief Get the HTTP verb that corresponds to the request's method as a string
 * 
 * @return const std::string& 
 */

const std::string& HTTP::Request::getMethod(void) const
{
    return _method;
}

/**
 * @brief Get the ressourceURI, i.e the original URI without the protocol and host information.
 To get the whole URI @see HTTP::Request::getURI.
 As an example, the ressourceURI of http://aurelienbrabant.fr/content/style.css is /content/style.css
 * 
 * @return const std::string& 
 */

const std::string& HTTP::Request::getResourceURI(void) const
{
    return _resourceURI;
}

/**
 * @brief Get the full URI of the request as typed in the browser's bar.
 * 
 * @return const std::string& 
 */

const std::string& HTTP::Request::getURI(void) const
{
    return _URI;
}

/**
 * @brief Get the protocol used and its version, under the form of <PROTOCOL>/<VERSION>. Given this project's purpose
 it should always be HTTP/1.1 for HTTP protocol version 1.1.
 * 
 * @return const std::string& 
 */


const std::string& HTTP::Request::getProtocol(void) const
{
    return _protocol;
}

/**
 * @brief Get the body of the request. If no body is provided, the empty string is returned.
 * 
 * @return const std::string& 
 */

const std::string HTTP::Request::getBody(void) const
{
    return body.str();
}

HTTP::Request& HTTP::Request::setServerBlock(ConfigItem* serverBlock)
{
    _serverBlock = serverBlock;

    return *this;
}

HTTP::Response::Response(void): _statusCode(HTTP::OK)
{
    setHeaderField("Server", "webserv/1.0");
}

/**
 * @brief Construct a new HTTP::Response::Response object
 * 
 * @param req 
 */

HTTP::Response::Response(const HTTP::Request& req): _statusCode(HTTP::OK), _req(req)
{
    setHeaderField("Server", "webserv/1.0");
}

/**
 * @brief Construct a new HTTP::Response::Response object
 * 
 * @param other 
 */

HTTP::Response::Response(const HTTP::Response& other): Message() 
{
    *this = other;
}

/**
 * @brief Assign data of rhs to lhs
 * 
 * @param rhs 
 * @return HTTP::Response& 
 */

HTTP::Response& HTTP::Response::operator=(const HTTP::Response& rhs)
{
    if (this != &rhs) {
        Message::operator=(rhs);
    }

    return *this;
}

/**
 * @brief Destroy the HTTP::Response::Response object
 * 
 */

HTTP::Response::~Response(void) 
{
}

/**
 * @brief Get the string representation of the response. The returned string can be directly sent back to the user agent.
 * 
 * @return std::string 
 */

std::string HTTP::Response::str(void)
{
    return _sendHeader() + _body;
}

/**
 * @brief Set the status code of the response using the HTTP::StatusCode enum.
 * 
 * @param statusCode 
 * @return HTTP::Response& 
 */

HTTP::Response& HTTP::Response::setStatus(StatusCode statusCode)
{
    _statusCode = statusCode;

    return *this;
}

/**
 * @brief Set the status code of the response using the value of the HTTP status code.
 * 
 * @param intStatusCode 
 * @return HTTP::Response& 
 */

HTTP::Response& HTTP::Response::setStatus(unsigned intStatusCode)
{
    _statusCode = static_cast<StatusCode>(intStatusCode);

    return *this;
}

/**
 * @brief Get the original request the response is answering too.
 * 
 * @return const HTTP::Request& 
 */

const HTTP::Request& HTTP::Response::getReq(void) const
{
    return _req;
}

/**
 * @brief Transform the map of header fields into an actually usuable HTTP header.
 * 
 * @return std::string 
 */

std::string HTTP::Response::_sendHeader(void)
{
    std::ostringstream oss;

    oss << "HTTP/1.1" << " " << _statusCode << " " << toStatusCodeString(_statusCode) << HTTP::CRLF;

    oss << "Content-Length: " << _body.size() << HTTP::CRLF;
    oss << _header.format();
    oss << HTTP::CRLF;

    return oss.str();
}

/**
 * @brief Send a file as the response's body, automatically figuring out its media type.
 Use of this member function will overwrite any previously set response's body.
 * 
 * @param filepath 
 * @return HTTP::Response& 
 */

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
    setHeaderField("Content-Type", _detectMediaType(filepath));

    return *this;
}

/**
 * @brief Send a string as the response's body. Media type text/plain is assumed.
 Use of this member function will overwrite any previously set response's body.
 *
 * @param s 
 * @return HTTP::Response& 
 */

HTTP::Response& HTTP::Response::send(const std::string& s)
{
    _body = s;
    
    return *this;
}

/**
 * @brief Append a string to the response's body, without overwriting anything.
 * 
 * @param s 
 * @return HTTP::Response& 
 */

HTTP::Response& HTTP::Response::append(const std::string& s)
{
    _body += s;
    
    return *this;
}

/**
 * @brief Easy and modular mediaType detection given the resource file extension (if any)
 * 
 * @param resource 
 * @return std::string 
 */

std::string HTTP::Response::_detectMediaType(const std::string &resource) const
{
    static MediaTypeEntry entries[] = {
        { "image/png", "png" },
        { "image/jpeg", "jpg|jpeg" },
        { "image/webp", "webp" },
        { "image/gif", "gif" },
        { "image/bmp", "bmp" },
        { "text/css", "css" },
        { "text/javascript", "js" },
        { "text/html", "html|htm" },
    };

    const std::string::size_type index = resource.find_last_of('.');
    const std::string ext = resource.substr(index + 1, resource.size() - index);

    for (size_t i = 0; i != sizeof(entries) / sizeof(*entries); ++i) {
        if (entries[i].extensions.find(ext) != std::string::npos) {
            return entries[i].mediaType;
        }
    }

    return "text/plain";
}