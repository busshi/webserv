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
