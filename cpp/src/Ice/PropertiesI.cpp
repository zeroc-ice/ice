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
    {
	return p->second;
    }
    else
    {
	return string();
    }
}

void
Ice::PropertiesI::setProperty(const string& key, const string& value)
{
    _properties[key] = value;
}

PropertiesPtr
Ice::PropertiesI::clone()
{
    int dummy = 0;
    PropertiesI* p = new PropertiesI(dummy, 0);
    p->_properties = _properties;
    return p;
}

Ice::PropertiesI::PropertiesI(int& argc, char* argv[])
{
    parseArgs(argc, argv);
}

Ice::PropertiesI::PropertiesI(int& argc, char* argv[], const string& file)
{
    load(file);
    parseArgs(argc, argv);
}

void
Ice::PropertiesI::parseArgs(int& argc, char* argv[])
{
    if (!argv)
    {
	return;
    }

    int idx = 1;
    while (idx < argc)
    {
	if (strncmp(argv[idx], "--Ice.", 6) == 0)
	{
	    string line = argv[idx];
	    for (int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;

	    if (line.find('=') == string::npos)
	    {
		line += "=1";
	    }
	    
	    parseLine(line.c_str() + 2);
	}
	else
	{
	    ++idx;
	}
    }
}

void
Ice::PropertiesI::load(const std::string& file)
{
    ifstream in(file.c_str());
    if (!in)
    {
	throw SystemException(__FILE__, __LINE__);
    }
    parse(in);
}

void
Ice::PropertiesI::parse(istream& in)
{
    char line[1024];
    while (in.getline(line, 1024))
    {
	parseLine(line);
    }
}

void
Ice::PropertiesI::parseLine(const char* line)
{
    const string delim = " \t";
    string s = line;
    
    string::size_type idx = s.find('#');
    if (idx != string::npos)
    {
	s.erase(idx);
    }
    
    idx = s.find_last_not_of(delim);
    if (idx != string::npos && idx + 1 < s.length())
    {
	s.erase(idx + 1);
    }
    
    string::size_type beg = s.find_first_not_of(delim);
    if (beg == string::npos)
    {
	return;
    }
    
    string::size_type end = s.find_first_of(delim + "=", beg);
    if (end == string::npos)
    {
	return;
    }
    
    string key = s.substr(beg, end - beg);
    
    end = s.find('=', end);
    if (end == string::npos)
    {
	return;
    }
    
    beg = s.find_first_not_of(delim + "=", end);
    if (beg == string::npos)
    {
	return;
    }
    
    end = s.length();
    
    string value = s.substr(beg, end - beg);
    
    setProperty(key, value);
}
