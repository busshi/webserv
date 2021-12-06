#include "http/message.hpp"

HTTP::Request::Request(void) {}

/**
 * @brief Construct a new HTTP::Request::Request object (with a client socket
 * only)
 *
 * @param csockFd
 */

HTTP::Request::Request(int csockFd, const HttpParser::Config& parserConf)
  : _parser(0)
  , _csockFd(csockFd)
  , _res(0)
{
    _parser = new HttpParser(parserConf);
}

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

HTTP::Request::~Request(void)
{
    delete _res;
    delete _parser;
}

bool
HTTP::Request::isDone(void) const
{
    return _parser->getState() == HttpParser::DONE;
}

bool
HTTP::Request::isBodyChunked(void) const
{
    return _parser->isBodyChunked();
}

HTTP::Response*
HTTP::Request::createResponse(void)
{
    return _res = new HTTP::Response(_csockFd);
}

HTTP::Response*
HTTP::Request::response(void)
{
    return _res;
}

int
HTTP::Request::getClientFd(void) const
{
    return _csockFd;
}

const std::string&
HTTP::Request::getLocation(void) const
{
    return _location;
}

void
HTTP::Request::setProtocol(const std::string& protocol)
{
    _protocol = protocol;
}

void
HTTP::Request::setMethod(const std::string& method)
{
    _method = method;
}

void
HTTP::Request::setLocation(const std::string& location)
{
    _location = location;
}

bool
HTTP::Request::parse(const std::string& data)
{
    _parser->parse(data, reinterpret_cast<uintptr_t>(this));

    return *_parser;
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
        _protocol = rhs._protocol;
    }
    return *this;
}

/**
 * @brief Get the HTTP verb that corresponds to the request's method as a
 * string
 *
 * @return const std::string&
 */

const std::string&
HTTP::Request::getMethod(void) const
{
    return _method;
}

/**
 * @brief Get the protocol used and its version, under the form of
 <PROTOCOL>/<VERSION>. Given this project's purpose it should always be
 HTTP/1.1 for HTTP protocol version 1.1.
 *
 * @return const std::string&
 */

const std::string&
HTTP::Request::getProtocol(void) const
{
    return _protocol;
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
