// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/DisableWarnings.h>
#include <IceE/Properties.h>
#include <IceE/StringUtil.h>
#include <IceE/Initialize.h>
#include <IceE/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(::Ice::Properties* p) { return p; }

string
Ice::Properties::getProperty(const string& key)
{
    IceUtil::Mutex::Lock sync(*this);

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
Ice::Properties::getPropertyWithDefault(const string& key, const string& value)
{
    IceUtil::Mutex::Lock sync(*this);

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
Ice::Properties::getPropertyAsInt(const string& key)
{
    return getPropertyAsIntWithDefault(key, 0);
}

Int
Ice::Properties::getPropertyAsIntWithDefault(const string& key, Int value)
{
    IceUtil::Mutex::Lock sync(*this);
    
    map<string, string>::const_iterator p = _properties.find(key);
    if(p != _properties.end())
    {
    	value = atoi(p->second.c_str());
    }

    return value;
}

PropertyDict
Ice::Properties::getPropertiesForPrefix(const string& prefix)
{
    IceUtil::Mutex::Lock sync(*this);

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
Ice::Properties::setProperty(const string& key, const string& value)
{
    if(key.empty())
    {
	return;
    }

    IceUtil::Mutex::Lock sync(*this);

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
Ice::Properties::getCommandLineOptions()
{
    IceUtil::Mutex::Lock sync(*this);

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
Ice::Properties::parseCommandLineOptions(const string& prefix, const StringSeq& options)
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
            
            parseLine(opt.substr(2)
#ifdef ICEE_HAS_WSTRING
                      , 0
#endif
                      );
        }
        else
        {
            result.push_back(opt);
        }
    }
    return result;
}

StringSeq
Ice::Properties::parseIceCommandLineOptions(const StringSeq& options)
{
    return parseCommandLineOptions("Ice", options);
}

void
Ice::Properties::load(const std::string& file)
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
	parseLine(line
#ifdef ICEE_HAS_WSTRING
                  , _converter
#endif
                  );
    }
    fclose(in);
}

PropertiesPtr
Ice::Properties::clone()
{
    IceUtil::Mutex::Lock sync(*this);
    return new Properties(this);
}

Ice::Properties::Properties(const Properties* p) :
    _properties(p->_properties)
#ifdef ICEE_HAS_WSTRING
    , _converter(p->_converter)
#endif
{
}

Ice::Properties::Properties(
#ifdef ICEE_HAS_WSTRING
    const StringConverterPtr& converter) : _converter(converter)
#else
    )
#endif
{
}

Ice::Properties::Properties(StringSeq& args, const PropertiesPtr& defaults
#ifdef ICEE_HAS_WSTRING
    , const StringConverterPtr& converter) : _converter(converter)
#else
    )
#endif
{
    if(defaults != 0)
    {
	_properties = defaults->getPropertiesForPrefix("");
    }

    StringSeq::iterator q = args.begin();
    if(q != args.end())
    {
        //
        // Use the first argument as the value for Ice.ProgramName. Replace
        // any backslashes in this value with forward slashes, in case this
        // value is used by the event logger.
        //
        string name = *q;
        replace(name.begin(), name.end(), '\\', '/');
	setProperty("Ice.ProgramName", name);
    }
    StringSeq tmp;

    bool loadConfigFiles = false;
    while(q != args.end())
    {
        string s = *q;
        if(s.find("--Ice.Config") == 0)
        {
            if(s.find('=') == string::npos)
            {
                s += "=1";
            }
            parseLine(s.substr(2)
#ifdef ICEE_HAS_WSTRING
                      , 0
#endif
                      );
	    loadConfigFiles = true;
        }
        else
        {
	    tmp.push_back(s);
        }
	++q;
    }
    args = tmp;

    if(!loadConfigFiles)
    {
	//
	// If Ice.Config is not set, load from ICE_CONFIG (if set)
	//
	loadConfigFiles = (_properties.find("Ice.Config") == _properties.end());
    }

    if(loadConfigFiles)
    {
	loadConfig();
    }

    args = parseIceCommandLineOptions(args);
}

void
Ice::Properties::parseLine(const string& line
#ifdef ICEE_HAS_WSTRING
        , const StringConverterPtr& converter
#endif
        )
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

#ifdef ICEE_HAS_WSTRING
    if(converter)
    {
        string tmp;
        converter->fromUTF8(reinterpret_cast<const Byte*>(key.data()),
                            reinterpret_cast<const Byte*>(key.data() + key.size()), tmp);
        key.swap(tmp);

        converter->fromUTF8(reinterpret_cast<const Byte*>(value.data()),
                            reinterpret_cast<const Byte*>(value.data() + value.size()), tmp);
        value.swap(tmp);
    }
#endif

    setProperty(key, value);
}

void
Ice::Properties::loadConfig()
{
    string value = getProperty("Ice.Config");

#ifndef _WIN32_WCE
    if(value.empty() || value == "1")
    {
        const char* s = getenv("ICE_CONFIG");
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

    setProperty("Ice.Config", value);
}
