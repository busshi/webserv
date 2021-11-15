#include "http/status.hpp"

HTTP::StatusCode HTTP::toStatusCode(unsigned int intStatusCode)
{
    return static_cast<HTTP::StatusCode>(intStatusCode);
}