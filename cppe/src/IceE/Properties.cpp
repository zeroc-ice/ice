// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/StringUtil.h>
#include <IceE/Properties.h>
#include <IceE/Initialize.h>
#include <IceE/LocalException.h>
#include <IceE/PropertyNames.h>

using namespace std;
using namespace IceE;
using namespace IceEInternal;

void
IceEInternal::incRef(::IceE::Properties* p)
{
    p->__incRef();
}

void
IceEInternal::decRef(::IceE::Properties* p)
{
    p->__decRef();
}

string
IceE::Properties::getProperty(const string& key)
{
    IceE::Mutex::Lock sync(*this);

    map<string, string>::const_iterator p = _properties.find(key);
    if(p != _properties.end())
    {
	return p->second;
    }
    else
    {
	return string();
    }
}

string
IceE::Properties::getPropertyWithDefault(const string& key, const string& value)
{
    IceE::Mutex::Lock sync(*this);

    map<string, string>::const_iterator p = _properties.find(key);
    if(p != _properties.end())
    {
	return p->second;
    }
    else
    {
	return value;
    }
}

Int
IceE::Properties::getPropertyAsInt(const string& key)
{
    return getPropertyAsIntWithDefault(key, 0);
}

Int
IceE::Properties::getPropertyAsIntWithDefault(const string& key, Int value)
{
    IceE::Mutex::Lock sync(*this);
    
    map<string, string>::const_iterator p = _properties.find(key);
    if(p != _properties.end())
    {
    	value = atoi(p->second.c_str());
    }

    return value;
}

PropertyDict
IceE::Properties::getPropertiesForPrefix(const string& prefix)
{
    IceE::Mutex::Lock sync(*this);

    PropertyDict result;
    map<string, string>::const_iterator p;
    for(p = _properties.begin(); p != _properties.end(); ++p)
    {
        if(prefix.empty() || p->first.compare(0, prefix.size(), prefix) == 0)
        {
            result.insert(*p);
        }
    }

    return result;
}

void
IceE::Properties::setProperty(const string& key, const string& value)
{
    if(key.empty())
    {
	return;
    }

    //
    // Check if the property is legal. (We write to stderr instead of
    // using a logger because no logger may be established at the time
    // the property is parsed.)
    //
    string::size_type dotPos = key.find('.');
    if(dotPos != string::npos)
    {
	string prefix = key.substr(0, dotPos);
	for(const char* const** i = IceEInternal::PropertyNames::validProps; *i != 0; ++i)
	{
	    string pattern(*i[0]);
	    dotPos = pattern.find('.');
	    assert(dotPos != string::npos);
	    string propPrefix = pattern.substr(0, dotPos);
	    if(propPrefix != prefix)
	    {
		continue;
	    }

	    bool found = false;
	    for(const char* const* j = *i; *j != 0 && !found; ++j)
	    {
		found = IceE::match(key, *j);
	    }
	    if(!found)
	    {
		fprintf(stderr, "warning: unknown property: %s\n", key.c_str());
	    }
	}
    }

    IceE::Mutex::Lock sync(*this);

    //
    // Set or clear the property.
    //
    if(!value.empty())
    {
	_properties[key] = value;
    }
    else
    {
	_properties.erase(key);
    }
}

StringSeq
IceE::Properties::getCommandLineOptions()
{
    IceE::Mutex::Lock sync(*this);

    StringSeq result;
    result.reserve(_properties.size());
    map<string, string>::const_iterator p;
    for(p = _properties.begin(); p != _properties.end(); ++p)
    {
	result.push_back("--" + p->first + "=" + p->second);
    }
    return result;
}

StringSeq
IceE::Properties::parseCommandLineOptions(const string& prefix, const StringSeq& options)
{
    string pfx = prefix;
    if(!pfx.empty() && pfx[pfx.size() - 1] != '.')
    {
	pfx += '.';
    }
    pfx = "--" + pfx;
    
    StringSeq result;
    StringSeq::size_type i;
    for(i = 0; i < options.size(); i++)
    {
        string opt = options[i];
        if(opt.find(pfx) == 0)
        {
            if(opt.find('=') == string::npos)
            {
                opt += "=1";
            }
            
            parseLine(opt.substr(2));
        }
        else
        {
            result.push_back(opt);
        }
    }
    return result;
}

StringSeq
IceE::Properties::parseIceCommandLineOptions(const StringSeq& options)
{
    return parseCommandLineOptions("IceE", options);
}

void
IceE::Properties::load(const std::string& file)
{
    FILE* in = fopen(file.c_str(), "r");
    if(!in)
    {
        FileException ex(__FILE__, __LINE__);
	ex.path = file;
        ex.error = getSystemErrno();
        throw ex;
    }

    char line[1024];
    while(fgets(line, 1024, in) != NULL)
    {
	parseLine(line);
    }
    fclose(in);
}

PropertiesPtr
IceE::Properties::clone()
{
    IceE::Mutex::Lock sync(*this);
    return new Properties(this);
}

IceE::Properties::Properties(const Properties* p) :
    _properties(p->_properties)
{
}

IceE::Properties::Properties()
{
    loadConfig();
}

IceE::Properties::Properties(StringSeq& args)
{
    StringSeq::iterator q = args.begin();
    if(q != args.end())
    {
        //
        // Use the first argument as the value for IceE.ProgramName. Replace
        // any backslashes in this value with forward slashes, in case this
        // value is used by the event logger.
        //
        string name = *q;
        replace(name.begin(), name.end(), '\\', '/');
	setProperty("IceE.ProgramName", name);
    }
    StringSeq tmp;
    while(q != args.end())
    {
        string s = *q;
        if(s.find("--IceE.Config") == 0)
        {
            if(s.find('=') == string::npos)
            {
                s += "=1";
            }
            parseLine(s.substr(2));
        }
        else
        {
	    tmp.push_back(s);
        }
	++q;
    }
    args = tmp;

    loadConfig();

    args = parseIceCommandLineOptions(args);
}

void
IceE::Properties::parseLine(const string& line)
{
    const string delim = " \t\r\n";
    string s = line;
    
    string::size_type idx = s.find('#');
    if(idx != string::npos)
    {
	s.erase(idx);
    }
    
    idx = s.find_last_not_of(delim);
    if(idx != string::npos && idx + 1 < s.length())
    {
	s.erase(idx + 1);
    }
    
    string::size_type beg = s.find_first_not_of(delim);
    if(beg == string::npos)
    {
	return;
    }
    
    string::size_type end = s.find_first_of(delim + "=", beg);
    if(end == string::npos)
    {
	return;
    }
    
    string key = s.substr(beg, end - beg);
    
    end = s.find('=', end);
    if(end == string::npos)
    {
	return;
    }
    ++end;

    string value;
    beg = s.find_first_not_of(delim, end);
    if(beg != string::npos)
    {
	end = s.length();
	value = s.substr(beg, end - beg);
    }
    
    setProperty(key, value);
}

void
IceE::Properties::loadConfig()
{
    string value = getProperty("IceE.Config");

#ifndef _WIN32_WCE
    if(value.empty() || value == "1")
    {
        const char* s = getenv("ICEE_CONFIG");
        if(s && *s != '\0')
        {
            value = s;
        }
    }
#endif

    if(!value.empty())
    {
        const string delim = " \t\r\n";
        string::size_type beg = value.find_first_not_of(delim);
        while(beg != string::npos)
        {
            string::size_type end = value.find(",", beg);
            string file;
            if(end == string::npos)
            {
                file = value.substr(beg);
                beg = end;
            }
            else
            {
                file = value.substr(beg, end - beg);
                beg = value.find_first_not_of("," + delim, end);
            }
            load(file);
        }
    }

    setProperty("IceE.Config", value);
}
