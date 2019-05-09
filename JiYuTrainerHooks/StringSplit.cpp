#include "stdafx.h"
#include "StringSplit.h"

void SplitString2(const std::string* s, std::vector<std::string>* v, const std::string& c)
{
	std::string::size_type pos1, pos2;
	pos2 = s->find(c);
	pos1 = 0;
	while (std::string::npos != pos2)
	{
		v->push_back(s->substr(pos1, pos2 - pos1));

		pos1 = pos2 + c.size();
		pos2 = s->find(c, pos1);
	}
	if (pos1 != s->length()) v->push_back(s->substr(pos1));
}
void SplitString2(const std::wstring *s, std::vector<std::wstring>* v, const std::wstring & c)
{
	std::wstring::size_type pos1, pos2;
	pos2 = s->find(c);
	pos1 = 0;
	while (std::wstring::npos != pos2)
	{
		v->push_back(s->substr(pos1, pos2 - pos1));

		pos1 = pos2 + c.size();
		pos2 = s->find(c, pos1);
	}
	if (pos1 != s->length())
		v->push_back(s->substr(pos1));
}

void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c)
{
	std::string::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	while (std::string::npos != pos2)
	{
		v.push_back(s.substr(pos1, pos2 - pos1));

		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())
		v.push_back(s.substr(pos1));
}
void SplitString(const std::wstring & s, std::vector<std::wstring>& v, const std::wstring & c)
{
	std::wstring::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	while (std::wstring::npos != pos2)
	{
		v.push_back(s.substr(pos1, pos2 - pos1));

		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())
		v.push_back(s.substr(pos1));
}
