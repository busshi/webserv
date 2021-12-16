#include "http/MessageParser.hpp"
#include "utils/string.hpp"

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <string>
#include <unistd.h>
#include <vector>

using HTTP::MessageParser;

/* Helpers */

void
MessageParser::_parseHeader(const Buffer<>& buf, uintptr_t paramLoc)
{
    std::string header = buf.str();
    std::vector<std::string> components = split(header);

    if (components.size() != 3) {
        std::cerr << "Expected 3 components in header, found "
                  << components.size() << "\n";
        return;
    }

    if (_config.onHeader) {
        _config.onHeader(components[0], components[1], components[2], paramLoc);
    }
}

/**
 * @brief Construct a new Http Parser:: Http Parser object
 *
 * This is the default constructor which takes no argument and as such,
 * registers no callbacks, making the parser almost useless.
 */

MessageParser::MessageParser(void)
  : _isChunked(false)
  , _currentChunkSize(-1)
  , _contentLength(-1)
  , _stopBodyParsing(false)
{
    memset(&_config, 0, sizeof(_config));
}

/**
 * @brief Construct a new Http Parser:: Http Parser object
 *
 * @param config The configuration object that contains the addresses of all the
 * callback functions.
 */

MessageParser::MessageParser(const MessageParser::Config& config,
                       MessageParser::State state)
  : _config(config)
  , _state(state)
  , _isChunked(false)
  , _currentChunkSize(-1)
  , _contentLength(-1)
  , _stopBodyParsing(false)
{}

/**
 * @brief Destroy the Http Parser:: Http Parser object
 */

MessageParser::~MessageParser(void) {}

void
MessageParser::stop(void)
{
    _state = DONE;
}

bool
MessageParser::hasDataQueued(void) const
{
    return _buf.size();
}

/**
 * @brief Get the current parser state.
 * @see MessageParser::State
 *
 * @return MessageParser::State
 */

MessageParser::State
MessageParser::getState(void) const
{
    return _state;
}

/**
 * @brief Whether or not the body is transmitted in a chunked manner. Checking
 * this is only relevant if the Transfer-Encoding header field has already been
 * parsed. By default, the message body is considered unchunked.
 *
 * @return true
 * @return false
 */

bool
MessageParser::isBodyChunked(void) const
{
    return _isChunked;
}

/**
 * @brief Reset the parser to a given state, making its internal buffer empty.
 *
 * @param state The state the parser will be reset to.
 * MessageParser::PARSING_HEADER is the default.
 */

void
MessageParser::reset(MessageParser::State state)
{
    _state = state;
}

void
MessageParser::stopBodyParsing(void)
{
    _stopBodyParsing = true;
}

/**
 * @brief THE member function you want to use. Given data, this function parses
 * the HTTP message, calling appropriate callbacks if properly configured.
 *
 * @param data
 * @param paramLoc
 */

void
MessageParser::parse(const char* data, size_t n, uintptr_t paramLoc)
{
    _buf += Buffer<>(data, n);

    // std::cout << "CONTENT OF IBUF:\n" << _ibuf.str();

    if (_state == PARSING_HEADER) {
        Buffer<>::size_type pos = _buf.find(CRLF);

        if (pos != Buffer<>::npos) {
            Buffer<> sub = _buf.subbuf(0, pos);

            _parseHeader(sub, paramLoc);

            _buf = _buf.subbuf(pos + 2);
            _state = PARSING_HEADER_FIELD_NAME;
        }
    }

    else if (_state == PARSING_HEADER_FIELD_NAME) {
        // body delimiter
        if (_buf.find(CRLF) == 0) {
            if (_config.onHeaderParsed) {
                _config.onHeaderParsed(paramLoc);
                if (static_cast<size_t>(-1) == _contentLength &&
                    !_config.parseFullBody) {
                    _contentLength = 0;
                }
            }

            _buf = _buf.subbuf(2);
            _state = PARSING_BODY;
        } else {
            Buffer<>::size_type pos = _buf.find(":");

            if (pos != Buffer<>::npos) {
                const std::string fieldName(_buf.subbuf(0, pos).str());

                if (_config.onHeaderFieldName) {
                    _config.onHeaderFieldName(fieldName, paramLoc);
                }

                _lastHeaderFieldName = fieldName;

                _state = PARSING_HEADER_FIELD_VALUE;
                _buf = _buf.subbuf(pos + 1);
            }
        }
    }

    else if (_state == PARSING_HEADER_FIELD_VALUE) {
        Buffer<>::size_type pos = _buf.find(CRLF);

        if (pos != Buffer<>::npos) {
            std::string s(_buf.subbuf(0, pos).str());

            // skip blanks that directly follow the colon
            Buffer<>::size_type begInd = 0;
            for (begInd = 0; begInd < s.size() && s[begInd] == ' ';)
                ++begInd;

            const std::string fieldValue(s.substr(begInd, pos - begInd));

            if (equalsIgnoreCase("TRANSFER-ENCODING", _lastHeaderFieldName) &&
                equalsIgnoreCase(fieldValue, "CHUNKED")) {
                _isChunked = true;
            } else if (equalsIgnoreCase(_lastHeaderFieldName,
                                        "CONTENT-LENGTH")) {
                _contentLength = parseInt(fieldValue, 10);
            }

            if (_config.onHeaderFieldValue) {
                _config.onHeaderFieldValue(fieldValue, paramLoc);
            }

            if (_config.onHeaderField) {
                _config.onHeaderField(
                  _lastHeaderFieldName, fieldValue, paramLoc);
            }

            _state = PARSING_HEADER_FIELD_NAME;
            _buf = _buf.subbuf(pos + 2);
        }
    }

    else if (_state == PARSING_BODY) {
        if (!_isChunked) {
            if (_contentLength == 0 || (!_buf.size() && _stopBodyParsing)) {
                _state = DONE;
                if (_config.onBodyParsed) {
                    _config.onBodyParsed(paramLoc);
                }

                return;
            }

            if (!_buf.size()) {
                return;
            }

            Buffer<> fragment = _buf.subbuf(0, _contentLength);
            if (_contentLength != static_cast<size_t>(-1)) {
                _contentLength -= fragment.size();
            }

            if (_config.onBodyFragment) {
                _config.onBodyFragment(fragment, paramLoc);
            }

            _buf = _buf.subbuf(fragment.size());
        }

        // chunked body
        else {
            // we want the chunk size first
            if (_currentChunkSize == static_cast<size_t>(-1)) {
                Buffer<>::size_type pos = _buf.find(CRLF);

                if (pos != std::string::npos) {
                    // negative check?
                    _currentChunkSize = parseInt(_buf.subbuf(0, pos).str(), 16);
                    _buf = _buf.subbuf(pos + 2);
                }
            } else {
                // decode the current chunk
                Buffer<>::size_type pos = _buf.rfind(CRLF);

                if (pos != Buffer<>::npos && pos >= _currentChunkSize) {
                    if (_currentChunkSize == 0) {
                        _state = DONE;
                        if (_config.onBodyUnchunked) {
                            _config.onBodyUnchunked(paramLoc);
                        }

                        return;
                    }

                    // never decode more than chunkSize even if there is
                    // more
                    Buffer<> decoded = _buf.subbuf(0, _currentChunkSize);

                    if (_config.onBodyChunk) {
                        _config.onBodyChunk(decoded, paramLoc);
                    }

                    _currentChunkSize = -1;
                    _buf = _buf.subbuf(pos + 2);
                }
            }
        }
    }
}
