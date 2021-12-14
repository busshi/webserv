#include <iomanip>

#include "Constants.hpp"
#include "config/ConfigItem.hpp"
#include "core.hpp"
#include "http/message.hpp"
#include "http/status.hpp"

using std::setw;

/**
 * @brief Construct a new HTTP::Request::Request object (with a client socket
 * only)
 *
 * @param csockFd
 */

HTTP::Request::Request(int csockFd, const HttpParser::Config& parserConf)
  : parser(0)
  , _method("UNKNOWN")
  , _location("UNKNOWN")
  , _protocol("UNKNOWN")
  , _csockFd(csockFd)
  , _res(0)
  , _block(0)
{
    parser = new HttpParser(parserConf);
    createResponse();
}

HTTP::Request::~Request(void)
{
    delete _res;
    delete parser;
}

ConfigItem*
HTTP::Request::getBlock(void) const
{
    return _block;
}

void
HTTP::Request::setBlock(ConfigItem* block)
{
    _block = block;
}

void
HTTP::Request::rewrite(const std::string& location)
{
    setLocation(location);
    setMethod("GET");

    processRequest(this);
}

bool
HTTP::Request::isDone(void) const
{
    return parser->getState() == HttpParser::DONE;
}

bool
HTTP::Request::isBodyChunked(void) const
{
    return parser->isBodyChunked();
}

HTTP::Response*
HTTP::Request::createResponse(void)
{
    return _res = new HTTP::Response(_csockFd);
}

HTTP::Response*&
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
HTTP::Request::parse(const char* data, size_t n)
{
    parser->parse(data, n, reinterpret_cast<uintptr_t>(this));

    return true;
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

std::ostream&
HTTP::Request::log(std::ostream& os) const
{
    std::string method = toUpperCase(getMethod());
    unsigned code = HTTP::toStatusCode(_res->getStatus());
    const char *methodColor, *statusColor;

    if (method == "GET") {
        methodColor = GREEN;
    } else if (method == "POST") {
        methodColor = ORANGE;
    } else if (method == "DELETE") {
        methodColor = RED;
    } else {
        methodColor = BOLD;
    }

    if (code >= 200 && code <= 299) {
        statusColor = GREEN;
    }

    else if (code >= 300 && code <= 399) {
        statusColor = CLR;
    }

    else if (code >= 400 && code <= 499) {
        statusColor = RED;
    }

    else if (code >= 500 && code <= 599) {
        statusColor = PURPLE;
    }

    os << methodColor << std::left << setw(12) << method << CLR << " "
       << setw(50) << getLocation() << statusColor << setw(8) << code << CLR
       << setw(10);
    if (_res->getStatus() == REQUEST_TIMEOUT) {
        os << BOLD << RED << "TIMEOUT" << CLR;
    } else {
        os << timer.getElapsed() << "ms" << CLR;
    }

    return os;
}
