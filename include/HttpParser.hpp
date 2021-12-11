#pragma once
#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <string>
#define CRLF "\r\n"

class HttpParser
{
    HttpParser(const HttpParser&);
    HttpParser& operator=(HttpParser&);

    void _parseHeader(const std::string& header, uintptr_t paramLoc);

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
        void (*onBodyChunk)(const std::string& decodedData, uintptr_t paramLoc);
        void (*onBodyUnchunked)(uintptr_t param);
        void (*onBodyFragment)(const std::string& fragment, uintptr_t paramLoc);
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
    std::ostringstream _ibuf;

    size_t _currentChunkSize; // set to -1 (the highest integer possible for
                              // that type) when not there.

    std::string _lastHeaderFieldName;
    size_t _contentLength;
    bool _stopBodyParsing;

  public:
    HttpParser(void);
    HttpParser(const Config& config, State state = PARSING_HEADER);
    ~HttpParser(void);

    State getState(void) const;
    bool isBodyChunked(void) const;

    void reset(State state = PARSING_HEADER);
    void stop(void);
    void stopBodyParsing(void);

    operator bool(void) const { return _state != DONE; }

    void parse(const std::string& data, uintptr_t paramLoc = 0);
};