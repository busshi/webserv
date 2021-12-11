#pragma once
#include "Buffer.hpp"
#include <sstream>
#include <stdint.h>

namespace HTTP {

class FormDataParser
{
  private:
    FormDataParser(const FormDataParser& other);
    FormDataParser& operator=(const FormDataParser& rhs);

  public:
    enum State
    {
        PARSING_BOUNDARY,
        PARSING_ENTRY_HEADER_FIELD_NAME,
        PARSING_ENTRY_HEADER_FIELD_VALUE,
        PARSING_ENTRY_BODY,
        DONE
    };

    struct CallbackList
    {
        // empty filename means that it's not a file
        void (*onBoundary)(uintptr_t param);
        void (*onEntryHeaderFieldName)(const std::string& name,
                                       uintptr_t param);
        void (*onEntryHeaderFieldValue)(const std::string& value,
                                        uintptr_t param);
        void (*onEntryHeaderField)(const std::string& name,
                                   const std::string& value,
                                   uintptr_t param);
        void (*onEntryHeaderParsed)(uintptr_t param);
        void (*onEntryBodyFragment)(const Buffer<>& fragment, uintptr_t param);
        void (*onEntryBodyParsed)(uintptr_t param);
        void (*onFinalBoundary)(uintptr_t param);
    };

  private:
    Buffer<> _buf;
    std::string _currentName, _boundary, _lastHeaderFieldName;
    size_t _fieldCount;
    CallbackList _callbacks;
    State _state;

  public:
    FormDataParser(const std::string& boundary, const CallbackList& callbacks);
    ~FormDataParser(void);

    void parse(const char* data, size_t n, uintptr_t param = 0);

    const std::string& getBoundary(void) const;
    State getState(void) const;
    size_t getFieldCount(void) const;
};

}
