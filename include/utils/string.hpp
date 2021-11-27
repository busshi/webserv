#pragma once
#include <string>
#include <vector>

std::vector<std::string>
split(const std::string& s, const std::string& set = "\t\n\r\v ");

std::string
trim(const std::string& s);

std::string
expandVar(const std::string& path);

bool
isIPv4(const std::string& s);

std::string
toLowerCase(const std::string& s);

long long
parseInt(const std::string& s, int radix);