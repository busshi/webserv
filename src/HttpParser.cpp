#include "HttpParser.hpp"

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

static std::string
trim(const std::string& s)
{
    std::string::size_type bpos = 0, epos = s.size() - 1;

    while (isspace(s[bpos]))
        ++bpos;
    while (epos >= bpos && isspace(s[epos]))
        --epos;

    return s.substr(bpos, epos - bpos + 1);
}

/**
 * @brief Helper method for working with split. Returns the index where any of
 * the characters in set first occurs in s.
 *
 * @param s The string to search in
 * @param set A set of characters that are matchable
 * @param pos The position to start the search at in s
 * @return std::string::size_type the index where the first occurence of any
 * character in set is found, std::string::npos is there is not.
 */

static std::string::size_type
splitFindOneOfSet(const std::string& s,
                  const std::string& set,
                  std::string::size_type pos = 0)
{
    for (std::string::size_type i = pos; i != s.size(); ++i) {
        if (set.find(s[i]) != std::string::npos) {
            return i;
        }
    }

    return std::string::npos;
}

/**
 * @brief splits a std::string into one or several strings, each character of
 * set being a possible delimiter.
 *
 * @param s The string to split
 * @param set A string which each of its characters can be a delimiter to split
 * s
 * @return std::vector<std::string>  A vector of string that holds the split.
 */

static std::vector<std::string>
split(const std::string& s, const std::string& set = "\t\n\r\v ")
{
    std::vector<std::string> v;
    std::string::size_type bpos = 0, fpos = 0;

    while (bpos < s.size()) {
        while (set.find(s[bpos]) != std::string::npos) {
            ++bpos;
        }
        if (bpos == s.size()) {
            break;
        }
        fpos = splitFindOneOfSet(s, set, bpos);
        if (fpos == std::string::npos) {
            fpos = s.size();
        }
        v.push_back(s.substr(bpos, fpos - bpos));
        bpos = fpos;
    }

    return v;
}

static std::string
toUpperCase(const std::string& s)
{
    std::string us = s;

    std::transform(us.begin(), us.end(), us.begin(), ::toupper);

    return us;
}

/**
 * @brief parse a string-encoded integer expressed in base radix, such as 2 <=
 * radix <= 36.
 *
 * @param s
 * @param radix
 * @return long long
 */

static long long
parseInt(const std::string& s, int radix = 10)
{
    static const std::string charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    long long parsed = 0;
    bool isNeg = false;
    std::string trimmed = trim(s);

    std::string::const_iterator sbit = trimmed.begin();

    while (sbit != trimmed.end() && (*sbit == '+' || *sbit == '-')) {
        if (*sbit == '-') {
            isNeg = !isNeg;
        }
        ++sbit;
    }

    while (sbit != trimmed.end()) {
        std::string::size_type pos = charset.find(toupper(*sbit));

        if (pos == std::string::npos) {
            break;
        }

        parsed = parsed * radix + pos;
        ++sbit;
    }

    return parsed * (isNeg ? -1 : 1);
}

/* Helpers */

void
HttpParser::_parseHeader(const std::string& header, uintptr_t paramLoc)
{
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

HttpParser::HttpParser(void)
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

HttpParser::HttpParser(const HttpParser::Config& config,
                       HttpParser::State state)
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

HttpParser::~HttpParser(void) {}

void
HttpParser::stop(void)
{
    _state = DONE;
}

/**
 * @brief Get the current parser state.
 * @see HttpParser::State
 *
 * @return HttpParser::State
 */

HttpParser::State
HttpParser::getState(void) const
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
HttpParser::isBodyChunked(void) const
{
    return _isChunked;
}

/**
 * @brief Reset the parser to a given state, making its internal buffer empty.
 *
 * @param state The state the parser will be reset to.
 * HttpParser::PARSING_HEADER is the default.
 */

void
HttpParser::reset(HttpParser::State state)
{
    _state = state;
}

void
HttpParser::stopBodyParsing(void)
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
HttpParser::parse(const std::string& data, uintptr_t paramLoc)
{
    if (!data.empty()) {
        _ibuf.seekp(0, std::ios::end);
        _ibuf.write(data.c_str(), data.size());
    }

    // std::cout << "CONTENT OF IBUF:\n" << _ibuf.str();

    if (_state == PARSING_HEADER) {
        const std::string s(_ibuf.str());
        std::string::size_type pos = s.find(CRLF);

        if (pos != std::string::npos) {
            std::string sub = s.substr(0, pos);

            _parseHeader(sub, paramLoc);

            _ibuf.str(s.substr(pos + 2, std::string::npos));
            _state = PARSING_HEADER_FIELD_NAME;
        }
    }

    else if (_state == PARSING_HEADER_FIELD_NAME) {
        const std::string s(_ibuf.str());

        // body delimiter
        if (s.find(CRLF) == 0) {
            if (_config.onHeaderParsed) {
                _config.onHeaderParsed(paramLoc);
                if (static_cast<size_t>(-1) == _contentLength &&
                    !_config.parseFullBody) {
                    _contentLength = 0;
                }
            }

            _ibuf.str(s.substr(2));
            _state = PARSING_BODY;
        } else {
            std::string::size_type pos = s.find(':');

            if (pos != std::string::npos) {
                const std::string fieldName(s.substr(0, pos));

                if (_config.onHeaderFieldName) {
                    _config.onHeaderFieldName(fieldName, paramLoc);
                }

                _lastHeaderFieldName = fieldName;

                _state = PARSING_HEADER_FIELD_VALUE;
                _ibuf.str(s.substr(pos + 1));
            }
        }
    }

    else if (_state == PARSING_HEADER_FIELD_VALUE) {
        const std::string s(_ibuf.str());
        std::string::size_type pos = s.find(CRLF);

        if (pos != std::string::npos) {
            // skip blanks that directly follow the colon
            std::string::size_type begInd = 0;
            for (begInd = 0; begInd < s.size() && s[begInd] == ' ';)
                ++begInd;

            const std::string fieldValue(s.substr(begInd, pos - begInd));

            // field specific settings
            std::string fnup = toUpperCase(_lastHeaderFieldName);

            if (fnup == "TRANSFER-ENCODING" &&
                toUpperCase(fieldValue) == "CHUNKED") {
                _isChunked = true;
            } else if (fnup == "CONTENT-LENGTH") {
                _contentLength = parseInt(fieldValue);
            }

            if (_config.onHeaderFieldValue) {
                _config.onHeaderFieldValue(fieldValue, paramLoc);
            }

            if (_config.onHeaderField) {
                _config.onHeaderField(
                  _lastHeaderFieldName, fieldValue, paramLoc);
            }

            _state = PARSING_HEADER_FIELD_NAME;
            _ibuf.str(s.substr(pos + 2, std::string::npos));
        }
    }

    else if (_state == PARSING_BODY) {
        if (!_isChunked) {
            if (_contentLength == 0 ||
                (_ibuf.str().empty() && _stopBodyParsing)) {
                _state = DONE;
                if (_config.onBodyParsed) {
                    _config.onBodyParsed(paramLoc);
                }

                return;
            }

            if (_ibuf.str().empty()) {
                return;
            }

            std::string s(_ibuf.str().substr(0, _contentLength));
            if (_contentLength != static_cast<size_t>(-1)) {
                _contentLength -= s.size();
            }

            if (_config.onBodyFragment) {
                _config.onBodyFragment(s, paramLoc);
            }

            _ibuf.str(_ibuf.str().substr(s.size()));
        }

        // chunked body
        else {
            std::string s(_ibuf.str());

            // we want the chunk size first
            if (_currentChunkSize == static_cast<size_t>(-1)) {
                std::string::size_type pos = s.find(CRLF);

                if (pos != std::string::npos) {
                    // negative check?
                    _currentChunkSize = parseInt(s.substr(0, pos), 16);
                    _ibuf.str(s.substr(pos + 2));
                }
            } else {
                // decode the current chunk
                std::string::size_type pos = s.rfind(CRLF);

                if (pos != std::string::npos && pos >= _currentChunkSize) {
                    if (_currentChunkSize == 0) {
                        _state = DONE;
                        if (_config.onBodyUnchunked) {
                            _config.onBodyUnchunked(paramLoc);
                        }

                        return;
                    }

                    // never decode more than chunkSize even if there is
                    // more
                    std::string decoded = s.substr(0, _currentChunkSize);

                    if (_config.onBodyChunk) {
                        _config.onBodyChunk(decoded, paramLoc);
                    }

                    _currentChunkSize = -1;
                    _ibuf.str(s.substr(pos + 2));
                }
            }
        }
    }
}
