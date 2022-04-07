#include <iomanip>
#include <unistd.h>

#include "Constants.hpp"
#include "config/ConfigItem.hpp"
#include "core.hpp"
#include "http/message.hpp"
#include "http/status.hpp"

using HTTP::MessageParser;
using HTTP::Request;
using std::setw;
using std::string;

/**
 * @brief Construct a new Request::Request object (with a client socket
 * only)
 *
 * @param csockFd
 */

Request::Request(int csockFd, const MessageParser::Config& parserConf)
  : parser(0)
  , _method("UNKNOWN")
  , _location("UNKNOWN")
  , _protocol("UNKNOWN")
  , _origLocation("UNKNOWN")
  , _csockFd(csockFd)
  , _resourceFd(-1)
  , _res(0)
  , _block(0)
  , _bodySize(0)
  , _maxBodySize(-1) // unlimited by default
{
    FD_SET(_csockFd, &select_rset);
    FD_SET(_csockFd, &select_wset);

    parser = new MessageParser(parserConf);
}

Request::~Request(void)
{
    dropFile();

    FD_CLR(_csockFd, &select_wset);
    FD_CLR(_csockFd, &select_rset);
    close(_csockFd);

    delete parser;
}

ConfigItem*
Request::getBlock(void) const
{
    return _block;
}

void
Request::setBlock(ConfigItem* block)
{
    _block = block;
}

unsigned long long
Request::getCurrentBodySize(void) const
{
    return _maxBodySize;
}

void
Request::setCurrentBodySize(unsigned long long size)
{
    _bodySize = size;
}

unsigned long long
Request::getMaxBodySize(void) const
{
    return _maxBodySize;
}

void
Request::setMaxBodySize(unsigned long long size)
{
    _maxBodySize = size;
}

void
Request::rewrite(const string& location)
{
    setLocation(location);
    setMethod("GET");

    processRequest(this);
}

bool
Request::isDone(void) const
{
    return _resourceFd == -1 && parser->getState() == MessageParser::DONE;
}

bool
Request::isBodyChunked(void) const
{
    return parser->isBodyChunked();
}

HTTP::Response&
Request::res(void)
{
    return _res;
}

int
Request::getClientFd(void) const
{
    return _csockFd;
}

const string&
Request::getLocation(void) const
{
    return _location;
}

const std::string&
Request::getQueryString(void) const
{
    return _queryString;
}

void
Request::setProtocol(const string& protocol)
{
    _protocol = protocol;
}

void
Request::setMethod(const string& method)
{
    _method = method;
}

void
Request::setLocation(const string& location)
{
    _location = location;
}

void
Request::setOriginalLocation(const string& origLocation)
{
    _origLocation = origLocation;
}

void
Request::setQueryString(const std::string& queryString)
{
    _queryString = queryString;
}

bool
Request::parse(const char* data, size_t n)
{
    return parser->parse(data, n, reinterpret_cast<uintptr_t>(this));
}

/**
 * @brief Get the HTTP verb that corresponds to the request's method as a
 * string
 *
 * @return const string&
 */

const string&
Request::getMethod(void) const
{
    return _method;
}

/**
 * @brief Get the protocol used and its version, under the form of
 <PROTOCOL>/<VERSION>. Given this project's purpose it should always be
 HTTP/1.1 for HTTP protocol version 1.1.
 *
 * @return const string&
 */

const string&
Request::getProtocol(void) const
{
    return _protocol;
}

const string&
Request::getOriginalLocation(void) const
{
    return _origLocation;
}

std::ostream&
Request::log(std::ostream& os) const
{
    string method = toUpperCase(getMethod());
    unsigned code = HTTP::toStatusCode(_res.getStatus());
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
       << setw(50) << getOriginalLocation() << statusColor << setw(8) << code
       << CLR;
    if (_res.getStatus() == REQUEST_TIMEOUT) {
        os << BOLD << RED << setw(10) << "TIMEOUT" << CLR;
    } else {
        os << setw(10) << (timer.getElapsed() / 1000) << "secs." << CLR;
    }

    return os;
}

void
Request::grabFile(int fd)
{
    FD_SET(fd, &select_rset);
    _resourceFd = fd;
}

void
Request::dropFile(void)
{
    if (_resourceFd != -1) {
        FD_CLR(_resourceFd, &select_rset);
        close(_resourceFd);
        _resourceFd = -1;
    }
}

int
Request::getFile(void) const
{
    return _resourceFd;
}

/**
 * Clears the Request object, making it ready to receive a new request.
 * Parser's internal buffer is not emptied to support pipelining.
 */

void
Request::clear(void)
{
    _res.clear();
    dropFile();
    timer.reset();
    body.clear();
    parser->reset(MessageParser::PARSING_HEADER);

    _bodySize = 0;
    _maxBodySize = -1;
    _block = 0;
    _header.clear();
}
