#include "http/method.hpp"
#include "utils/string.hpp"

using std::string;

bool
HTTP::isMethodImplemented(const string& methodName)
{
    static const char* supportedMethods[] = { "GET", "POST", "DELETE" };
    string normalizedMethod = toUpperCase(methodName);

    for (size_t i = 0;
         i != sizeof(supportedMethods) / sizeof(*supportedMethods);
         ++i) {
        if (normalizedMethod == supportedMethods[i]) {
            return true;
        }
    }

    return false;
}
