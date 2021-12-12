#include "http/FormDataParser.hpp"
#include "http/message.hpp"
#include <cctype>
#include <unistd.h>

HTTP::FormDataParser::FormDataParser(const std::string& boundary,
                                     const CallbackList& callbacks)
  : _boundary(boundary)
  , _callbacks(callbacks)
  , _state(PARSING_BOUNDARY)
{}

HTTP::FormDataParser::~FormDataParser(void) {}

/**
 * Return the minimum of the 4 buffer positions
 */

static Buffer<>::size_type
min4(Buffer<>::size_type pos1,
     Buffer<>::size_type pos2,
     Buffer<>::size_type pos3,
     Buffer<>::size_type pos4)
{
    Buffer<>::size_type min = -1;

    if (pos1 < min)
        min = pos1;
    if (pos2 < min)
        min = pos2;
    if (pos3 < min)
        min = pos3;
    if (pos4 < min)
        min = pos4;

    return min;
}

void
HTTP::FormDataParser::parse(const char* data, size_t n, uintptr_t param)
{
    _buf += Buffer<>(data, n);

    if (_state == PARSING_BOUNDARY) {
        Buffer<>::size_type pos = _buf.find("--" + _boundary + CRLF);

        if (pos != Buffer<>::npos) {
            _state = PARSING_ENTRY_HEADER_FIELD_NAME;
            if (_callbacks.onBoundary) {
                _callbacks.onBoundary(param);
            }
            _buf = _buf.subbuf(pos + _boundary.size() + 4);
        }
    }

    else if (_state == PARSING_ENTRY_HEADER_FIELD_NAME) {

        if (_buf.find(CRLF) == 0) {
            _state = PARSING_ENTRY_BODY;
            if (_callbacks.onEntryHeaderParsed) {
                _callbacks.onEntryHeaderParsed(param);
            }

            _buf = _buf.subbuf(2);

            return;
        }

        Buffer<>::size_type pos = _buf.find(":");

        if (pos != Buffer<>::npos) {
            std::string fieldName = _lastHeaderFieldName =
              _buf.subbuf(0, pos).str();

            _state = PARSING_ENTRY_HEADER_FIELD_VALUE;

            if (_callbacks.onEntryHeaderFieldName) {
                _callbacks.onEntryHeaderFieldName(fieldName, param);
            }

            _buf = _buf.subbuf(pos + 1);
        }
    }

    else if (_state == PARSING_ENTRY_HEADER_FIELD_VALUE) {
        Buffer<>::size_type pos = _buf.find(CRLF);

        if (pos != Buffer<>::npos) {
            std::string s(_buf.subbuf(0, pos).str());
            Buffer<>::size_type begPos = 0;

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

            _buf = _buf.subbuf(pos + 2);
        }
    }

    else if (_state == PARSING_ENTRY_BODY) {
        Buffer<>::size_type pos1, pos2;

        pos1 = _buf.find(std::string(CRLF) + "--" + _boundary + "--");

        // encountered an immediate final boundary
        if (pos1 == 0) {
            _state = DONE;

            if (_callbacks.onEntryBodyParsed) {
                _callbacks.onEntryBodyParsed(param);
            }

            if (_callbacks.onFinalBoundary) {
                _callbacks.onFinalBoundary(param);
            }

            _buf = _buf.subbuf(pos1 + _boundary.size() + 6);

            return;
        }

        pos2 = _buf.find(std::string(CRLF) + "--" + _boundary);

        // encountered an immediate boundary
        if (pos2 == 0) {
            _state = PARSING_ENTRY_HEADER_FIELD_NAME;

            if (_callbacks.onEntryBodyParsed) {
                _callbacks.onEntryBodyParsed(param);
            }

            _buf = _buf.subbuf(pos2 + _boundary.size() + 6);

            return;
        }

        Buffer<>::size_type pos3 =
          _buf.findDangling(std::string(CRLF) + "--" + _boundary);
        Buffer<>::size_type pos4 =
          _buf.findDangling(std::string(CRLF) + "--" + _boundary + "--");

        // entry body to parse
        Buffer<> fragment = _buf.subbuf(0, min4(pos1, pos2, pos3, pos4));

        if (_callbacks.onEntryBodyFragment) {
            _callbacks.onEntryBodyFragment(fragment, param);
        }

        _buf = _buf.subbuf(fragment.size());
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

HTTP::FormDataParser::State
HTTP::FormDataParser::getState(void) const
{
    return _state;
}
