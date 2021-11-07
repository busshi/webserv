#include <cctype>
#include <cstdlib>
#include <string>

static std::string
expandVariable(const std::string& path, size_t& pos)
{
    size_t beginPos = ++pos;

    while (pos < path.size() && isalnum(path[pos])) {
        ++pos;
    }

    const char* envVar = getenv(path.substr(beginPos, pos - beginPos).c_str());

    return envVar ? envVar : "";
}

std::string
expandVar(const std::string& path)
{
    std::string expanded;
    size_t pos = 0, fIndex = 0;

    while (pos < path.size() &&
           (fIndex = path.find('$', pos)) != std::string::npos) {
        expanded += path.substr(pos, fIndex - pos);
        /* The two add statements are separated because fIndex is mutated by
         * expandVariable: otherwise order of evaluation matters and is
         * unspecified */
        expanded += expandVariable(path, fIndex);
        pos = fIndex;
    }
    expanded += path.substr(pos, path.size() - pos);

    return expanded;
}
