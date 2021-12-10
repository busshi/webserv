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
        std::string::size_type pos = s.find(_boundary);

        if (pos != std::string::npos) {
            _state = PARSING_ENTRY_HEADER_FIELD_NAME;
            if (_callbacks.onBoundary) {
                _callbacks.onBoundary(param);
            }
            _ibuf.str(s.substr(pos + _boundary.size()));
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
        std::string::size_type pos;

        // encountered an immediate boundary
        if ((pos = s.find(_boundary)) == 0) {

            // ending boundary
            if (s.size() - _boundary.size() >= 2 &&
                s[_boundary.size()] == '-' && s[_boundary.size() + 1] == '-') {
                _state = DONE;
                if (_callbacks.onFinalBoundary) {
                    _callbacks.onFinalBoundary(param);
                }
            }

            else {
                _state = PARSING_ENTRY_HEADER_FIELD_NAME;

                if (_callbacks.onBoundary) {
                    _callbacks.onBoundary(param);
                }
            }
        }

        // entry body to parse
        else {
            std::string fragment = s.substr(
              0, pos); // stop at boundary if it exists and is not immediate
            if (_callbacks.onEntryBodyFragment) {
                _callbacks.onEntryBodyFragment(fragment, param);
            }

            _ibuf.str(s.substr(fragment.size()));
        }
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
