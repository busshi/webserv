#include "Constants.hpp"
#include "utils/ErrorPageGenerator.hpp"
#include "logger/Logger.hpp"
#include <sstream>

ErrorPageGenerator::ErrorPageGenerator( void ) {}

ErrorPageGenerator::ErrorPageGenerator( ErrorPageGenerator const & src ) {

	*this = src;
}

ErrorPageGenerator&	ErrorPageGenerator::operator=( ErrorPageGenerator const & rhs ) {

    if (this != &rhs) {    }

    return *this;
}

ErrorPageGenerator::~ErrorPageGenerator( void ) {}

std::string
ErrorPageGenerator::_replace(std::string in, std::string s1, std::string s2) {

    std::string     out;
    std::string     line;
    size_t          len = s1.size();
    std::istringstream  iss(in);

    while (getline(iss, line)) {

        for (size_t i = 0; i < line.size(); i++) {

                size_t  found = line.find(s1, i);

                if (found == std::string::npos) {

                    out += &line[i];
                    break;
                }
                else {

                    if (found > i) {

                        std::string subLine = line.substr(i, found - i);
                        out += subLine;
                        i += subLine.size();
                    }
                    out += s2;
                    i += len - 1;
                }
        }
        out += "\n";
    }
    return out;
}

void
ErrorPageGenerator::generate( std::string file, std::string code, std::string msg, std::string sentence ) {

    std::ifstream   ifs(file.c_str());
    std::string     res;
    std::string     line;
    std::string     out;

    while (getline(ifs, line)) {

        res += line;
        res += "\n";
    }

    ifs.close();

    out = _replace(res, "ERROR_CODE", code);
    out = _replace(out, "ERROR_MESSAGE", msg);
    out = _replace(out, "ERROR_SENTENCE", sentence);


    std::ofstream   ofs(ERROR_PAGE);
    ofs << out;
    ofs.close();
}

std::string
ErrorPageGenerator::checkErrorPage( std::string defaultPage, std::string code, std::string errorMsg, std::string errorSentence ) {


    struct stat s;

    if (stat(defaultPage.c_str(), &s) == 0) {

        glogger << Logger::DEBUG << Logger::getTimestamp() << " Default error page exists. Using " << defaultPage << "\n";
        return defaultPage;
    }
    else {

        glogger << Logger::DEBUG << Logger::getTimestamp() << " Default error page does not exist. Using webserv default error page\n";
        generate(ERROR_SAMPLE, code, errorMsg, errorSentence);
        return ERROR_PAGE;
    }
}
