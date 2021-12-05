#include "http/message.hpp"
#include "logger/Logger.hpp"
#include <iostream>
#include <string>

#define GET_REQ(loc) *reinterpret_cast<HTTP::Request*>(loc);

using std::string;

void
onHeaderField(const string& name, const string& value, uintptr_t requestLoc)
{
    HTTP::Request& req = GET_REQ(requestLoc);

    req.setHeaderField(name, value);
}

void
onHeaderParsed(uintptr_t requestLoc)
{
    HTTP::Request& req = GET_REQ(requestLoc);

    std::cout << "Header has been parsed." << std::endl;
    req.parseHeaderFromData();
}

void
onBodyFragment(const string& fragment, uintptr_t requestLoc)
{
    (void)requestLoc;

    std::cout << "Body fragment: " << fragment << std::endl;
}

void
onBodyChunk(const string& chunk, uintptr_t requestLoc)
{
    (void)requestLoc;

    std::cout << "chunk: " << chunk << std::endl;
}
