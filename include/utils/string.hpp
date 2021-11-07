#pragma once
#include <string>
#include <vector>

std::vector<std::string>
split(const std::string& s, unsigned char c);

std::string
trim(const std::string& s);

std::string
expandVar(const std::string& path);
