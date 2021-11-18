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
    std::map<std::string, std::string>::const_iterator cit =_header.find(name);

    return cit != _header.end() ? cit->second : "";
}

/**
 * @brief Set the value of a given header field.
 * 
 * @param name The name of the field, which is case *insensitive*
 * @param value The value to assign to the field. If the field already has a value, it gets replaced by the new one.
 */

void HTTP::Message::setHeaderField(const std::string& name, const std::string& value)
{
    _header[name] = value;
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

/**
 * @brief Construct a new HTTP::Request::Request object
 * 
 * @param rawData Basically a stream of bytes, here passed as a std::string. 
 It is usually the data sent by the user agent through the client socket.
 */

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

const std::string& HTTP::Request::getBody(void) const
{
    return _body;
}

/**
 * @brief Construct a new HTTP::Response::Response object
 * 
 * @param req 
 */

HTTP::Response::Response(const HTTP::Request& req): _statusCode(HTTP::OK), _req(req)
{
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

std::string HTTP::Response::str(void) const
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

std::string HTTP::Response::_sendHeader(void) const
{
    std::ostringstream oss;

    oss << _statusCode << " " << toStatusCodeString(_statusCode) << HTTP::CRLF;

    for (HTTP::Header::const_iterator cit = _header.begin(); cit != _header.end(); ++cit) {
        oss << cit->first << ": " << cit->second << HTTP::CRLF;
    }
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