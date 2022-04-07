#pragma once

#include <string>

class ErrorPageGenerator
{

  public:
    ErrorPageGenerator(void);
    ErrorPageGenerator(ErrorPageGenerator const& src);
    ErrorPageGenerator& operator=(ErrorPageGenerator const& rhs);
    ~ErrorPageGenerator(void);

    std::string checkErrorPage(std::string defaultPage,
                               std::string code,
                               std::string errorMsg,
                               std::string errorSentence);

  private:
    std::string _replace(std::string in, std::string s1, std::string s2);
    void _generate(std::string file,
                   std::string code,
                   std::string msg,
                   std::string sentence);
};
