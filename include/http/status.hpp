#pragma once
#include <string>

namespace HTTP {
    enum StatusCode {

        /* 100 - INFO */
        CONTINUE = 100,
        SWITCHING_PROTOCOLS = 101,
        EARLY_HINTS = 103,

        /* 200 - SUCCESS */
        OK = 200,
        CREATED = 201,
        ACCEPTED = 202,
        NON_AUTHORITATIVE_INFO = 203,
        NO_CONTENT = 204,
        RESET_CONTENT = 205,
        PARTIAL_CONTENT = 206,

        /* 300 - REDIRECTION */
        MULTIPLE_CHOICES = 300,
        MOVED_PERMANENTLY = 301,
        FOUND = 302,
        SEE_OTHER = 303,
        NOT_MODIFED = 304,
        USE_PROXY = 305, 
        SWITCH_PROXY = 306,
        TEMPORARY_REDIRECT = 307,
        PERMANENT_REDIRECT = 308,
        TOO_MANY_REDIRECTS = 310,

        /* 400 - ERROR */
        BAD_REQUEST = 400,
        UNAUTHORIZED = 401,
        PAYMENT_REQUIRED = 402,
        FORBIDDEN = 403,
        NOT_FOUND = 404,
        METHOD_NOT_ALLOWED = 405,
        NOT_ACCEPTABLE = 406,
        PROXY_AUTH_REQUIRED = 407,
        REQUEST_TIMEOUT = 408,
        CONFLICT = 409,
        GONE = 410,
        LENGTH_REQUIRED = 411,
        PRECONDITION_FAILED = 412,
        REQUEST_PAYLOAD_TOO_LARGE = 413,
        REQUEST_URI_TOO_LONG = 414,
        UNSUPPORTED_MEDIA_TYPE = 415,
        REQUESTED_RANGE_UNSATISFIABLE = 416,
        EXPECTATION_FAILED = 417,
        MISDIRECTED_REQUEST = 421,
        UNPROCESSABLE_ENTITY = 422,
        TOO_EARLY = 425,
        UPGRADE_REQUIRED = 426,
        PRECONDITON_REQUIRED = 428,
        TOO_MANY_REQUESTS = 429,
        REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
        UNAVAILABLE_LEGAL = 451,
        UNRECOVERABLE_ERROR = 456,

        /* 500 - Server */

        INTERNAL_SERVER_ERROR = 500,
        NOT_IMPLEMENTED = 501,
        BAD_GATEWAY = 502,
        SERVICE_UNAVAILABLE = 503,
        GATEWAY_TIMEOUT = 504,
        HTTP_VERSION_NOT_SUPPORTED = 505,
        VARIANT_ALSO_NEGOTIATES = 506,
        BANDWIDTH_LIMIT_EXCEEDED = 509,
        NOT_EXTENDED = 510,
        NET_AUTH_REQUIRED = 511,
    };

    StatusCode toStatusCode(unsigned intStatusCode);
    std::string toStatusCodeString(StatusCode statusCode);
    std::string toStatusCodeString(unsigned intStatusCode);
}