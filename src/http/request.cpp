#include "http/message.hpp"

HTTP::Request::Request(void) {}

/**
 * @brief Construct a new HTTP::Request::Request object
 *
 * @param rawData Basically a stream of bytes, here passed as a std::string.
 It is usually the data sent by the user agent through the client socket.
 */

HTTP::Request::Request(int csockFd, const std::string& headerRawData)
  : _csockFd(csockFd)
{
    _parseHeader(headerRawData);
    remContentLength = 0;
}

HTTP::Request&
HTTP::Request::_parseHeader(const std::string& headerData)
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

HTTP::Request::~Request(void) {}

/**
 * @brief Construct a new HTTP::Request::Request object
 *
 * @param other
 */

HTTP::Request::Request(const HTTP::Request& other)
  : Message()
{
    *this = other;
}

/**
 * @brief Copy data of rhs to lhs
 *
 * @param rhs
 * @return HTTP::Request&
 */

HTTP::Request&
HTTP::Request::operator=(const HTTP::Request& rhs)
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

const std::string&
HTTP::Request::getMethod(void) const
{
    return _method;
}

/**
 * @brief Get the ressourceURI, i.e the original URI without the protocol and
 host information. To get the whole URI @see HTTP::Request::getURI. As an
 example, the ressourceURI of http://aurelienbrabant.fr/content/style.css is
 /content/style.css
 *
 * @return const std::string&
 */

const std::string&
HTTP::Request::getResourceURI(void) const
{
    return _resourceURI;
}

/**
 * @brief Get the full URI of the request as typed in the browser's bar.
 *
 * @return const std::string&
 */

const std::string&
HTTP::Request::getURI(void) const
{
    return _URI;
}

/**
 * @brief Get the protocol used and its version, under the form of
 <PROTOCOL>/<VERSION>. Given this project's purpose it should always be HTTP/1.1
 for HTTP protocol version 1.1.
 *
 * @return const std::string&
 */

const std::string&
HTTP::Request::getProtocol(void) const
{
    return _protocol;
}

/**
 * @brief Get the body of the request. If no body is provided, the empty string
 * is returned.
 *
 * @return const std::string&
 */

const std::string
HTTP::Request::getBody(void) const
{
    return body.str();
}

HTTP::Request&
HTTP::Request::setServerBlock(ConfigItem* serverBlock)
{
    _serverBlock = serverBlock;

    return *this;
}

ConfigItem*
HTTP::Request::getServerBlock(void) const
{
    return _serverBlock;
}
