// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/PropertiesI.h>
#include <Ice/LocalException.h>
#include <fstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

string
Ice::PropertiesI::getProperty(const string& key)
{
    map<string, string>::const_iterator p = _properties.find(key);
    if (p != _properties.end())
	return p->second;
    else
	return string();
}

void
Ice::PropertiesI::setProperty(const string& key, const string& value)
{
    _properties[key] = value;
}

Ice::PropertiesI::PropertiesI()
{
}

Ice::PropertiesI::PropertiesI(const string& file)
{
    load(file);
}

void
Ice::PropertiesI::load(const std::string& file)
{
    ifstream in(file.c_str());
    if (!in)
	throw SystemException(__FILE__, __LINE__);
    parse(in);
}

void
Ice::PropertiesI::parse(istream& in)
{
    const string delim = " \t";

    char line[1024];
    while (in.getline(line, 1024))
    {
	string s = line;

	string::size_type idx = s.find('#');
	if (idx != string::npos)
	    s.erase(idx);

	idx = s.find_last_not_of(delim);
	if (idx != string::npos && idx + 1 < s.length())
	    s.erase(idx + 1);

	string::size_type beg = s.find_first_not_of(delim);
	if (beg == string::npos)
	    continue;
	
	string::size_type end = s.find_first_of(delim + "=", beg);
	if (end == string::npos)
	    continue;

	string key = s.substr(beg, end - beg);

	end = s.find('=', end);
	if (end == string::npos)
	    continue;

	beg = s.find_first_not_of(delim + "=", end);
	if (beg == string::npos)
	    continue;
	
	end = s.length();

	string value = s.substr(beg, end - beg);
	
	setProperty(key, value);
    }
}
