#include "http/message.hpp"
#include "utils/Logger.hpp"
#include "utils/string.hpp"
#include <fstream>
#include <sstream>
#include <string>

/* HTTP::Message */

/**
 * @brief retrieve the value of a given header field.
 *
 * @param name The name of the field, which is case *insensitive*
 * @return std::string
 */

std::string
HTTP::Message::getHeaderField(const std::string& name) const
{
    return _header.getField(name);
}

/**
 * @brief Set the value of a given header field.
 *
 * @param name The name of the field, which is case *insensitive*
 * @param value The value to assign to the field. If the field already has a
 * value, it gets replaced by the new one.
 */

void
HTTP::Message::setHeaderField(const std::string& name, const std::string& value)
{
    _header.setField(name, value);
}

/**
 * @brief Construct a new HTTP::Message::Message object
 *
 */

HTTP::Message::Message(void) {}

/**
 * @brief Destroy the HTTP::Message::Message object
 *
 */

HTTP::Message::~Message(void) {}

/**
 * @brief Construct a new HTTP::Message::Message object
 *
 * @param other
 */

HTTP::Message::Message(const Message& other)
{
    *this = other;
}

/**
 * @brief Copy data of rhs into lhs
 *
 * @param rhs
 * @return HTTP::Message&
 */

HTTP::Message&
HTTP::Message::operator=(const Message& rhs)
{
    if (this != &rhs) {
        _header = rhs._header;
    }

    return *this;
}
