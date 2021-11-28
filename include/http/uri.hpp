#pragma once
#include <string>

/* Everything related to the HTTP protocol */
namespace HTTP {
    std::string URIencode(const std::string& decodedURI);
}