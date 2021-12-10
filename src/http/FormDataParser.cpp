#include "http/FormDataParser.hpp"
#include "http/message.hpp"
#include <cctype>

HTTP::FormDataParser::FormDataParser(const std::string& boundary,
                                     const CallbackList& callbacks)
  : _boundary(boundary)
  , _callbacks(callbacks)
  , _state(PARSING_BOUNDARY)
{}

HTTP::FormDataParser::~FormDataParser(void) {}

void
HTTP::FormDataParser::parse(const std::string& data, uintptr_t param)
{
    if (!data.empty()) {
        _ibuf.seekp(0, std::ios::end);
        _ibuf.write(data.data(), data.size());
    }

    const std::string s = _ibuf.str();

    if (_state == PARSING_BOUNDARY) {
        std::string::size_type pos = s.find(_boundary + CRLF);

        if (pos != std::string::npos) {
            _state = PARSING_ENTRY_HEADER_FIELD_NAME;
            if (_callbacks.onBoundary) {
                _callbacks.onBoundary(param);
            }
            _ibuf.str(s.substr(pos + _boundary.size() + 2));
        }
    }

    else if (_state == PARSING_ENTRY_HEADER_FIELD_NAME) {

        if (s.find(CRLF) == 0) {
            _state = PARSING_ENTRY_BODY;
            if (_callbacks.onEntryHeaderParsed) {
                _callbacks.onEntryHeaderParsed(param);
            }

            return;
        }

        std::string::size_type pos = s.find(':');

        if (pos != std::string::npos) {
            std::string fieldName = _lastHeaderFieldName = s.substr(0, pos);

            _state = PARSING_ENTRY_HEADER_FIELD_VALUE;

            if (_callbacks.onEntryHeaderFieldName) {
                _callbacks.onEntryHeaderFieldName(fieldName, param);
            }

            _ibuf.str(s.substr(pos));
        }
    }

    else if (_state == PARSING_ENTRY_HEADER_FIELD_VALUE) {
        std::string::size_type pos = s.find(CRLF);

        if (pos != std::string::npos) {
            std::string::size_type begPos = 1;

            while (isspace(s[begPos]))
                ++begPos;

            std::string fieldValue = s.substr(begPos, pos - begPos);

            _state = PARSING_ENTRY_HEADER_FIELD_NAME;

            if (_callbacks.onEntryHeaderFieldValue) {
                _callbacks.onEntryHeaderFieldValue(fieldValue, param);
            }

            if (_callbacks.onEntryHeaderField) {
                _callbacks.onEntryHeaderField(
                  _lastHeaderFieldName, fieldValue, param);
            }

            _ibuf.str(s.substr(pos + 2));
        }
    }

    else if (_state == PARSING_ENTRY_BODY) {
        std::string::size_type pos1, pos2;

        pos1 = s.find(_boundary + "--" + CRLF);

        // encountered an immediate final boundary
        if (pos1 == 0) {
            _state = DONE;

            if (_callbacks.onFinalBoundary) {
                _callbacks.onFinalBoundary(param);
            }

            return;
        }

        pos2 = s.find(_boundary + CRLF);

        // encountered an immediate boundary
        if (pos2 == 0) {
            _state = PARSING_ENTRY_HEADER_FIELD_NAME;

            if (_callbacks.onBoundary) {
                _callbacks.onBoundary(param);
            }

            return;
        }

        // entry body to parse
        std::string fragment = s.substr(
          0,
          std::min(pos1,
                   pos2)); // stop at nearest boundary (if there is one)
        if (_callbacks.onEntryBodyFragment) {
            _callbacks.onEntryBodyFragment(fragment, param);
        }

        _ibuf.str(s.substr(fragment.size()));
    }
}

const std::string&
HTTP::FormDataParser::getBoundary(void) const
{
    return _boundary;
}

size_t
HTTP::FormDataParser::getFieldCount(void) const
{
    return _fieldCount;
}
