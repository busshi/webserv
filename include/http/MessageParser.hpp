#pragma once
#include "Buffer.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <string>
#define CRLF "\r\n"

namespace HTTP {

class MessageParser
{
    MessageParser(const MessageParser&);
    MessageParser& operator=(MessageParser&);

    bool _parseHeader(const Buffer<>& buf, uintptr_t paramLoc);

    /* public type definitions */

  public:
    /**
     * @brief Configuration object that is used to initialize the parser.
     * Every member prefixed by 'on' is a callback function, that is: a
     * function that will be called back when a parser event happens. For
     * example, the onHeaderFieldName callback function will be called each
     * time a new header field (including its name and value) is parsed.
     *
     * Because it is often needed to deal with an external object inside the
     * callback, EACH callback takes a void pointer as its last parameter, as
     * a way to pass the address of an additional object to the callback. The
     * address of that object must be passed to the parse function when it is
     * called.
     */

    struct Config
    {
        void (*onHeader)(const std::string& method,
                         const std::string& loc,
                         const std::string& protocol,
                         uintptr_t paramLoc);
        void (*onHeaderField)(const std::string& name,
                              const std::string& value,
                              uintptr_t paramLoc);
        void (*onHeaderFieldName)(const std::string& name, uintptr_t paramLoc);
        void (*onHeaderFieldValue)(const std::string& value,
                                   uintptr_t paramLoc);
        void (*onHeaderParsed)(uintptr_t paramLoc);
        void (*onBodyChunk)(const Buffer<>& chunk, uintptr_t paramLoc);
        void (*onBodyUnchunked)(uintptr_t param);
        void (*onBodyFragment)(const Buffer<>& fragment, uintptr_t paramLoc);
        void (*onBodyParsed)(uintptr_t param);
        bool parseFullBody;
    };

    enum State
    {
        PARSING_HEADER, // the first line of the request/response
        PARSING_HEADER_FIELD_NAME,
        PARSING_HEADER_FIELD_VALUE,
        PARSING_BODY,
        DONE
    };

    /* private attributes */

  private:
    Config _config;
    State _state;
    bool _isChunked;
    Buffer<> _buf;

    size_t _currentChunkSize; // set to -1 (the highest integer possible for
                              // that type) when not there.

    std::string _lastHeaderFieldName;
    size_t _contentLength;
    bool _stopBodyParsing;

  public:
    class IllFormedException : public std::runtime_error
    {
      public:
        IllFormedException(const std::string& s) throw();
    };

    MessageParser(void);
    MessageParser(const Config& config, State state = PARSING_HEADER);
    ~MessageParser(void);

    State getState(void) const;
    bool isBodyChunked(void) const;

    void reset(State state = PARSING_HEADER);
    void stop(void);
    void stopBodyParsing(void);

    operator bool(void) const { return _state != DONE; }

    bool parse(const char* data, size_t n, uintptr_t paramLoc = 0);
    bool hasDataQueued(void) const;
};
}
