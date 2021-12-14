#pragma once
#include <string>
#include <vector>

std::vector<std::string>
split(const std::string& s, const std::string& set = "\t\n\r\v ");

std::string
trim(const std::string& s, const std::string& set = "\t\n\r\v ");

std::string
trimTrailing(const std::string& s, const std::string& set);

std::string
expandVar(const std::string& path);

bool
isIPv4(const std::string& s);

std::string
toLowerCase(const std::string& s);

std::string
toUpperCase(const std::string& s);

long long
parseInt(const std::string& s, int radix);

std::string
ntos(long long n, int radix = 10, bool lower = false);

bool
hasFileExtension(const std::string& path, const std::string& fileExtension);

bool
equalsIgnoreCase(const std::string& s1, const std::string& s2);

bool
isNumber(const std::string& s);
