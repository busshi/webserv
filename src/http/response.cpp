#include "http/message.hpp"
#include "utils/Logger.hpp"
#include <sstream>

HTTP::Response::Response(int csock)
  : _statusCode(HTTP::OK)
  , _csock(csock)
{
    setHeaderField("Server", "webserv/1.0");
}

int
HTTP::Response::getClientSocket(void) const
{
    return _csock;
}

/**
 * @brief Construct a new HTTP::Response::Response object
 *
 * @param other
 */

HTTP::Response::Response(const HTTP::Response& other)
  : Message()
{
    *this = other;
}

/**
 * @brief Assign data of rhs to lhs
 *
 * @param rhs
 * @return HTTP::Response&
 */

HTTP::Response&
HTTP::Response::operator=(const HTTP::Response& rhs)
{
    if (this != &rhs) {
        Message::operator=(rhs);
        _csock = rhs._csock;
    }

    return *this;
}

/**
 * @brief Destroy the HTTP::Response::Response object
 *
 */

HTTP::Response::~Response(void) {}

/**
 * @brief Get the string representation of the response. The returned string can
 * be directly sent back to the user agent.
 *
 * @return std::string
 */

std::string
HTTP::Response::str(void)
{
    return "";
}

/**
 * @brief Set the status code of the response using the HTTP::StatusCode enum.
 *
 * @param statusCode
 * @return HTTP::Response&
 */

HTTP::Response&
HTTP::Response::setStatus(StatusCode statusCode)
{
    _statusCode = statusCode;

    return *this;
}

/**
 * @brief Set the status code of the response using the value of the HTTP status
 * code.
 *
 * @param intStatusCode
 * @return HTTP::Response&
 */

HTTP::Response&
HTTP::Response::setStatus(unsigned intStatusCode)
{
    _statusCode = static_cast<StatusCode>(intStatusCode);

    return *this;
}

HTTP::Header&
HTTP::Message::header(void)
{
    return _header;
}

/**
 * @brief Transform the map of header fields into an actually usuable HTTP
 * header.
 *
 * @return std::string
 */

std::string
HTTP::Response::formatHeader(void) const
{
    std::ostringstream oss;

    oss << "HTTP/1.1"
        << " " << _statusCode << " " << toStatusCodeString(_statusCode) << CRLF;

    oss << _header.format();
    oss << CRLF;

    return oss.str();
}

HTTP::Header&
HTTP::Response::header(void)
{
    return _header;
}

/**
 * @brief Send a file as the response's body, automatically figuring out its
 media type. Use of this member function will overwrite any previously set
 response's body.
 *
 * @param filepath
 * @return HTTP::Response&
 */

HTTP::Response&
HTTP::Response::sendFile(const std::string& filepath)
{
    std::ifstream ifs(filepath.c_str(), std::ios::binary | std::ios::in);
    std::ostringstream oss;

    if (ifs) {
        body.clear();
        body.append(ifs);
    }

    setHeaderField("Content-Type", _detectMediaType(filepath));
    oss << body.size();
    setHeaderField("Content-Length", oss.str());

    return *this;
}

/**
 * @brief Send a string as the response's body. Media type text/plain is
 assumed. Use of this member function will overwrite any previously set
 response's body.
 *
 * @param s
 * @return HTTP::Response&
 */

HTTP::Response&
HTTP::Response::send(const std::string& s)
{
    body.clear();
    body.append(s);

    return *this;
}

/**
 * @brief Append a string to the response's body, without overwriting anything.
 *
 * @param s
 * @return HTTP::Response&
 */

HTTP::Response&
HTTP::Response::append(const std::string& s)
{
    body.append(s);

    return *this;
}

/**
 * @brief Easy and modular mediaType detection given the resource file extension
 * (if any)
 *
 * @param resource
 * @return std::string
 */

std::string
HTTP::Response::_detectMediaType(const std::string& resource) const
{
    static MediaTypeEntry entries[] = {
        { "image/png", "png" },      { "image/jpeg", "jpg|jpeg" },
        { "image/webp", "webp" },    { "image/gif", "gif" },
        { "image/bmp", "bmp" },      { "text/css", "css" },
        { "text/javascript", "js" }, { "text/html", "html|htm" },
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

BinBuffer
HTTP::Response::format(void)
{
    BinBuffer bbuf;

    return bbuf;
}

