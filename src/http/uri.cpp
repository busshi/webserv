#include "http/uri.hpp"
#include "utils/string.hpp"
#include <cctype>
#include <stdexcept>

using std::string;

namespace HTTP {

string
urlDecode(const string& url)
{
    string s;
    string::size_type pos = 0;

    while (1) {
        string::size_type n = url.find_first_of("+%", pos);

        s += url.substr(pos, n - pos);

        if (n == string::npos) {
            break;
        }

        if (url[n] == '+') {
            s += " ";
            pos = n + 1;
        } else if (url[n] == '%') {
            if (url.size() - n - 1 < 2 ||
                !(isxdigit(url[n + 1]) && isxdigit(url[n + 2]))) {
                throw std::runtime_error(
                  "Ill-formed url: percent encoding needs two hex digits");
            }
            s += std::string(1, parseInt(url.substr(n + 1, 2), 16));
            pos = n + 3;
        }
    }

    return s;
}

}
