#pragma once
#include "http/message.hpp"
#include "http/status.hpp"
#include <stdexcept>

namespace HTTP {

class Exception : public std::runtime_error
{
  private:
    HTTP::Request* _req;
    StatusCode _statusCode;

  public:
    Exception(HTTP::Request* req,
              StatusCode statusCode,
              const std::string& hint = "No hint") throw();
    Exception(const Exception& other);
    ~Exception() throw();

    HTTP::Request* req(void) const;

    StatusCode status(void) const;
};

}
