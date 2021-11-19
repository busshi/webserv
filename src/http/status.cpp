#include "http/status.hpp"

static std::string codeStrings[][100] = {
    /* 1xx */
    {
        "Waiting for request body",
        "Switching protocols: accepted by the server",
        "",
        "Early hints",
    },

    /* 2xx */
    {
        "Request processed successfully",
        "Resource created successfully",
        "Request accepted but is still processed",
        "Non-authoritative information: transforming proxy has modified origin server's response",
        "Request processed, nothing to return",
        "Request processed, document view reload required",
        "Resource partly delivered",
    },

    /* 3xx */
    {

        "Multiple resources match the request",
        "Resource permanently moved",
        "Found at another URI",
        "See other URI",
        "Resource has not been modified",
        "Use the given proxy to access the resource",
        "Use proxy one subsequent requests",
        "Redirected temporarily: subsequent requests will use the old URI",
        "Redirected permanently",
        "",
        "Too many redirects (stuck in a direct loop?)",
    },

    /* 4xx */
    {
        "Bad request",
        "Unauthorized",
        "Payment required",
        "Forbidden",
        "Not found",
        "Method not allowed",
        "Not acceptable",
        "Proxy authentication required",
        "Request timeout",
        "Conflict",
        "Gone: resource does no longer exist",
        "Content length missing",
        "Precondition failed",
        "Payload too large",
        "URI too long",
        "Media type not supported",
        "Range not satisfiable",
        "Expectation failed",
        "",
        "",
        "",
        "Misdirected request",
        "Unprocessable entity",
        "",
        "",
        "Too early",
        "Upgrade required",
        "",
        "Precondition required",
        "Too many requests",
        "",
        "Request Header Fields Too Large",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "Unavailable for legal reasons",
        "",
        "",
        "",
        "",
        "Unrecoverable error",
    },

    {
        "Internal server error",
        "Not implemented",
        "Bad gateway",
        "Service unavailable",
        "Gateway timeout",
        "HTTP version not supported",
        "Variant also negotiates",
        "Bandwidth limit exceeded",
        "Not extended",
        "Network authentication required",
    }
};

/**
 * @brief Return the HTTP::StatusCode enum member corresponding to the integer status passed as a parameter.
 * 
 * @param intStatusCode an integer that represents a valid HTTP status code, that is any number between 100 and 599.
 If the integer falls outside of this range, the returned value should not be considered to be meaningful.
 *
 * @return HTTP::StatusCode 
 */

HTTP::StatusCode HTTP::toStatusCode(unsigned intStatusCode)
{
    return static_cast<HTTP::StatusCode>(intStatusCode);
}

/**
 * @brief Return the integer corresponding to the HTTP:StatusCode enum member passed as a parameter.
 * 
 * @param statusCode
 * @return unsigned
 */

unsigned HTTP::toStatusCode(HTTP::StatusCode statusCode)
{
    return static_cast<int>(statusCode);
}

/**
 * @brief Return a message describing the HTTP::StatusCode enum member passed as a parameter.
 * 
 * @param statusCode 
 * @return std::string 
 */

std::string HTTP::toStatusCodeString(HTTP::StatusCode statusCode)
{   
    size_t typeIndex = static_cast<int>(statusCode) / 100 - 1;

    return codeStrings[typeIndex][static_cast<int>(statusCode) - (typeIndex + 1) * 100];
}

/**
 * @brief Return a message describing the HTTP integer status code passed as a parameter.
 * 
 * @param intStatusCode
 * @return std::string 
 */

std::string HTTP::toStatusCodeString(unsigned intStatusCode)
{
    size_t typeIndex = intStatusCode / 100 - 1;

    return codeStrings[typeIndex][intStatusCode - (typeIndex + 1) * 100];
}