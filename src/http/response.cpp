#include "http/message.hpp"
#include "utils/Logger.hpp"
#include "utils/os.hpp"
#include <sstream>

using HTTP::Response;
using HTTP::StatusCode;

void
Response::_initHeader(void)
{
    setHeaderField("Server", "webserv/1.0");
    setHeaderField("Date", getDate(0));
    setHeaderField("Content-type", "text/plain");
}

Response::Response(int csock)
  : _statusCode(HTTP::OK)
  , _csock(csock)
{
    _initHeader();
}

/**
 * @brief Construct a new Response::Response object
 *
 * @param other
 */

Response::Response(const Response& other)
  : Message()
{
    *this = other;
}

/**
 * @brief Destroy the Response::Response object
 *
 */

Response::~Response(void) {}

/**
 * @brief Assign data of rhs to lhs
 *
 * @param rhs
 * @return Response&
 */

Response&
Response::operator=(const Response& rhs)
{
    if (this != &rhs) {
        Message::operator=(rhs);
        _csock = rhs._csock;
    }

    return *this;
}

/**
 * @brief Set the status code of the response using the StatusCode enum.
 *
 * @param statusCode
 * @return Response&
 */

Response&
Response::setStatus(StatusCode statusCode)
{
    _statusCode = statusCode;

    return *this;
}

/**
 * @brief Set the status code of the response using the value of the HTTP status
 * code.
 *
 * @param intStatusCode
 * @return Response&
 */

Response&
Response::setStatus(unsigned intStatusCode)
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
Response::formatHeader(void) const
{
    std::ostringstream oss;

    oss << "HTTP/1.1"
        << " " << _statusCode << " " << toStatusCodeString(_statusCode) << CRLF;

    oss << _header.format();
    oss << CRLF;

    return oss.str();
}

/**
 * @brief Send a string as the response's body. Media type text/plain is
 assumed. Use of this member function will overwrite any previously set
 response's body.
 *
 * @param s
 * @return Response&
 */

Response&
Response::send(const std::string& s)
{
    body = s;

    setHeaderField("Content-Length", ntos(s.size()));

    return *this;
}

/**
 * @brief Append a string to the response's body, without overwriting anything.
 *
 * @param s
 * @return Response&
 */

Response&
Response::append(const std::string& s)
{
    unsigned long long cl = parseInt(getHeaderField("Content-Length"), 10);

    setHeaderField("Content-Length", ntos(cl + s.size()));
    body += s;

    return *this;
}

/**
 * @brief Easy and modular mediaType detection given the resource file extension
 * (if any)
 *
 * @param resource
 * @return std::string
 */

void
Response::setContentType(const std::string& path)
{
    static MediaTypeEntry entries[] = { { "image/png", "png" },
                                        { "image/jpeg", "jpg|jpeg" },
                                        { "image/webp", "webp" },
                                        { "image/gif", "gif" },
                                        { "image/bmp", "bmp" },
                                        { "text/css", "css" },
                                        { "application/javascript", "js" },
                                        { "text/html", "html|htm" },
                                        { "application/pdf", "pdf" },
                                        { "image/svg+xml", "svg" },
                                        { "image/x-icon", "ico" },
                                        { "application/json", "json" },
                                        { "video/mp4", "mp4" },
                                        { "video/webm", "webm" },
                                        { "application/ogg", "ogg" } };

    const std::string::size_type index = path.find_last_of('.');
    const std::string ext = path.substr(index + 1, path.size() - index);

    for (size_t i = 0; i != sizeof(entries) / sizeof(*entries); ++i) {
        if (entries[i].extensions.find(ext) != std::string::npos) {
            setHeaderField("Content-Type", entries[i].mediaType);
            return;
        }
    }

    setHeaderField("Content-Type", "text/plain");
}

StatusCode
Response::getStatus(void) const
{
    return _statusCode;
}

void
Response::clear(void)
{
    data.clear();
    body.clear();
    _header.clear();
    _initHeader();
    _statusCode = HTTP::OK;
}

HTTP::Header&
Response::header()
{
    return _header;
}
