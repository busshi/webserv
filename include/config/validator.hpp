#pragma once
#include <string>

bool
validateAutoindex(const std::string& value, std::string& errorMsg);

bool
validateMethod(const std::string& value, std::string& errorMsg);

bool
validateIndex(const std::string& value, std::string& errorMsg);

bool
validateRoot(const std::string& value, std::string& errorMsg);

bool
validateListen(const std::string& value, std::string& errorMsg);

bool
validateSize(const std::string& value, std::string& errorMsg);

bool
validateLogLevel(const std::string& value, std::string& errorMsg);

bool
validateRedirect(const std::string& value, std::string& errorMsg);

bool
validateCgiPass(const std::string& value, std::string& errorMsg);

bool
validateNumber(const std::string& value, std::string& errorMsg);

bool
validateErrorPage(const std::string& value, std::string& errorMsg);

bool
validateBool(const std::string& value, std::string& errorMsg);
