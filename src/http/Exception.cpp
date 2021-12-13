#include "http/Exception.hpp"
#include "http/status.hpp"
#include <stdexcept>

HTTP::Exception::Exception(HTTP::Request* req,
                           HTTP::StatusCode statusCode,
                           const std::string& hint) throw()
  : std::runtime_error(toStatusCodeString(statusCode) + ": " + hint)
  , _req(req)
  , _statusCode(statusCode)
{}

HTTP::Exception::Exception(const HTTP::Exception& other)
  : std::runtime_error(other.what())
  , _req(other._req)
  , _statusCode(other._statusCode)
{}

HTTP::Exception::~Exception(void) throw() {}

HTTP::Request*
HTTP::Exception::req(void) const
{
    return _req;
}

HTTP::StatusCode
HTTP::Exception::status(void) const
{
    return _statusCode;
}
