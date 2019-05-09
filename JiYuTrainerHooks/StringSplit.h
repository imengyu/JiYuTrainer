#pragma once
#include <string>  
#include <vector>

void SplitString2(const std::string * s, std::vector<std::string>* v, const std::string & c);
void SplitString2(const std::wstring * s, std::vector<std::wstring>* v, const std::wstring & c);

void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c);
void SplitString(const std::wstring & s, std::vector<std::wstring>& v, const std::wstring & c);


