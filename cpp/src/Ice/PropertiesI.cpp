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
#include <Ice/Exception.h>
#include <fstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

std::set<string> Ice::PropertiesI::_argumentPrefixes;

string
Ice::PropertiesI::getProperty(const string& key)
{
    IceUtil::Mutex::Lock sync(*this);

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

string
Ice::PropertiesI::getPropertyWithDefault(const string& key, const string& value)
{
    IceUtil::Mutex::Lock sync(*this);

    map<string, string>::const_iterator p = _properties.find(key);
    if (p != _properties.end())
    {
	return p->second;
    }
    else
    {
	return value;
    }
}

StringSeq
Ice::PropertiesI::getProperties(const string& prefix)
{
    IceUtil::Mutex::Lock sync(*this);

    StringSeq result;
    map<string, string>::const_iterator p;
    for (p = _properties.begin(); p != _properties.end(); ++p)
    {
        if (prefix.empty() || p->first.find(prefix) == 0)
        {
            result.push_back(p->first);
            result.push_back(p->second);
        }
    }

    return result;
}

void
Ice::PropertiesI::setProperty(const string& key, const string& value)
{
    IceUtil::Mutex::Lock sync(*this);

    _properties[key] = value;
}

StringSeq
Ice::PropertiesI::getCommandLineOptions()
{
    IceUtil::Mutex::Lock sync(*this);

    StringSeq result;
    result.reserve(_properties.size());
    map<string, string>::const_iterator p;
    for (p = _properties.begin(); p != _properties.end(); ++p)
    {
	result.push_back("--" + p->first + "=" + p->second);
    }
    return result;
}

PropertiesPtr
Ice::PropertiesI::clone()
{
    IceUtil::Mutex::Lock sync(*this);

    int dummy = 0;
    PropertiesI* p = new PropertiesI(dummy, 0);
    p->_properties = _properties;
    return p;
}

void
Ice::PropertiesI::addArgumentPrefix(const std::string& prefix)
{
    _argumentPrefixes.insert(prefix);
}

Ice::PropertiesI::PropertiesI(int& argc, char* argv[])
{
    for (int i = 1; i < argc; ++i)
    {
	if (strncmp(argv[i], "--Ice.Config", 12) == 0)
	{
	    string line = argv[i];
	    if (line.find('=') == string::npos)
	    {
		line += "=1";
	    }
	    parseLine(line.substr(2));
	}
    }

    string file = getProperty("Ice.Config");

    if (file.empty() || file == "1")
    {
	const char* s = getenv("ICE_CONFIG");
	if (s && *s != '\0')
	{
	    file = s;
	}
    }

    if (!file.empty())
    {
	load(file);
    }

    parseArgs(argc, argv);
    setProperty("Ice.Config", file);
}

Ice::PropertiesI::PropertiesI(int& argc, char* argv[], const string& file)
{
    if (!file.empty())
    {
	load(file);
    }

    parseArgs(argc, argv);
    setProperty("Ice.Config", file);
}

void
Ice::PropertiesI::parseArgs(int& argc, char* argv[])
{
    if (!argv)
    {
	return;
    }

    if (argc > 0)
    {
	string name = getProperty("Ice.ProgramName");
	if (name.empty())
	{
	    setProperty("Ice.ProgramName", argv[0]);
	}
    }

    int idx = 1;
    while (idx < argc)
    {
	bool match = false;
	string arg = argv[idx];
	string::size_type beg = arg.find("--");
	if (beg == 0)
	{
            string rest = arg.substr(2);
            if (rest.find("Ice.") == 0)
            {
                match = true;
            }
            else
            {
                set<string>::const_iterator p;
                for (p = _argumentPrefixes.begin(); p != _argumentPrefixes.end(); ++p)
                {
                    if (rest.find((*p) + ".") == 0)
                    {
                        match = true;
                        break;
                    }
                }
            }

            if (match)
            {
                for (int i = idx ; i + 1 < argc ; ++i)
                {
                    argv[i] = argv[i + 1];
                }
                --argc;
                
                if (arg.find('=') == string::npos)
                {
                    arg += "=1";
                }
                
                parseLine(arg.substr(2));
            }
	}
	
	if (!match)
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
	SystemException ex(__FILE__, __LINE__);
	ex.error = getSystemErrno();
	throw ex;
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
Ice::PropertiesI::parseLine(const string& line)
{
    static const string delim = " \t\r\n";
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
    ++end;

    string value;
    beg = s.find_first_not_of(delim, end);
    if (beg != string::npos)
    {
	end = s.length();
	value = s.substr(beg, end - beg);
    }
    
    setProperty(key, value);
}
