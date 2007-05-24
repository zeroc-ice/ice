// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/StringUtil.h>
#include <Ice/PropertiesI.h>
#include <Ice/Initialize.h>
#include <Ice/LocalException.h>
#include <Ice/PropertyNames.h>
#include <Ice/Logger.h>
#include <fstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

string
Ice::PropertiesI::getProperty(const string& key)
{
    IceUtil::Mutex::Lock sync(*this);

    map<string, PropertyValue>::iterator p = _properties.find(key);
    if(p != _properties.end())
    {
        p->second.used = true;
        return p->second.value;
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

    map<string, PropertyValue>::iterator p = _properties.find(key);
    if(p != _properties.end())
    {
        p->second.used = true;
        return p->second.value;
    }
    else
    {
        return value;
    }
}

Int
Ice::PropertiesI::getPropertyAsInt(const string& key)
{
    return getPropertyAsIntWithDefault(key, 0);
}

Int
Ice::PropertiesI::getPropertyAsIntWithDefault(const string& key, Int value)
{
    IceUtil::Mutex::Lock sync(*this);
    
    map<string, PropertyValue>::iterator p = _properties.find(key);
    if(p != _properties.end())
    {
        p->second.used = true;
        istringstream v(p->second.value);
        if(!(v >> value) || !v.eof())
        {
            return 0;
        }
    }

    return value;
}

PropertyDict
Ice::PropertiesI::getPropertiesForPrefix(const string& prefix)
{
    IceUtil::Mutex::Lock sync(*this);

    PropertyDict result;
    map<string, PropertyValue>::iterator p;
    for(p = _properties.begin(); p != _properties.end(); ++p)
    {
        if(prefix.empty() || p->first.compare(0, prefix.size(), prefix) == 0)
        {
            p->second.used = true;
            result[p->first] = p->second.value;
        }
    }

    return result;
}

void
Ice::PropertiesI::setProperty(const string& key, const string& value)
{
    if(key.empty())
    {
        return;
    }

    //
    // Check if the property is legal.
    //
    LoggerPtr logger = getProcessLogger();
    string::size_type dotPos = key.find('.');
    string currentKey = key;
    if(dotPos != string::npos)
    {
        string prefix = currentKey.substr(0, dotPos);
        for(int i = 0 ; IceInternal::PropertyNames::validProps[i].properties != 0; ++i)
        {
            string pattern(IceInternal::PropertyNames::validProps[i].properties[0].pattern);
            
	    
            dotPos = pattern.find('.');

	    //
	    // Each top level prefix describes a non-empty
	    // namespace. Having a string without a prefix followed by a
	    // dot is an error.
	    //
            assert(dotPos != string::npos);
	    
            string propPrefix = pattern.substr(0, dotPos);
            if(propPrefix != prefix)
            {
                continue;
            }

            bool found = false;

            for(int j = 0; j < IceInternal::PropertyNames::validProps[i].length && !found; ++j)
            {
                const IceInternal::Property& prop = IceInternal::PropertyNames::validProps[i].properties[j];
                found = IceUtil::match(currentKey, prop.pattern);

                if(found && prop.deprecated)
                {
                    logger->warning("deprecated property: " + currentKey);
                    if(prop.deprecatedBy != 0)
                    {
                        currentKey = prop.deprecatedBy;
                    }
                }
            }
            if(!found)
            {
                logger->warning("unknown property: " + currentKey);
            }
        }
    }

    IceUtil::Mutex::Lock sync(*this);

    //
    // Set or clear the property.
    //
    if(!value.empty())
    {
        PropertyValue pv(value, false);
        map<string, PropertyValue>::const_iterator p = _properties.find(currentKey);
        if(p != _properties.end())
        {
            pv.used = p->second.used;
        }
        _properties[currentKey] = pv;
    }
    else
    {
        _properties.erase(currentKey);
    }
}

StringSeq
Ice::PropertiesI::getCommandLineOptions()
{
    IceUtil::Mutex::Lock sync(*this);

    StringSeq result;
    result.reserve(_properties.size());
    map<string, PropertyValue>::const_iterator p;
    for(p = _properties.begin(); p != _properties.end(); ++p)
    {
        result.push_back("--" + p->first + "=" + p->second.value);
    }
    return result;
}

StringSeq
Ice::PropertiesI::parseCommandLineOptions(const string& prefix, const StringSeq& options)
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
            
            parseLine(opt.substr(2), 0);
        }
        else
        {
            result.push_back(opt);
        }
    }
    return result;
}

StringSeq
Ice::PropertiesI::parseIceCommandLineOptions(const StringSeq& options)
{
    StringSeq args = options;
    for(const char** i = IceInternal::PropertyNames::clPropNames; *i != 0; ++i)
    {
        args = parseCommandLineOptions(*i, args);
    }
    return args;

}

void
Ice::PropertiesI::load(const std::string& file)
{
    ifstream in(file.c_str());
    if(!in)
    {
        FileException ex(__FILE__, __LINE__);
        ex.path = file;
        ex.error = getSystemErrno();
        throw ex;
    }

    char line[1024];
    while(in.getline(line, 1024))
    {
        parseLine(line, _converter);
    }
}

PropertiesPtr
Ice::PropertiesI::clone()
{
    IceUtil::Mutex::Lock sync(*this);
    return new PropertiesI(this);
}

set<string>
Ice::PropertiesI::getUnusedProperties()
{
    IceUtil::Mutex::Lock sync(*this);
    set<string> unusedProperties;
    for(map<string, PropertyValue>::const_iterator p = _properties.begin(); p != _properties.end(); ++p)
    {
        if(!p->second.used)
        {
            unusedProperties.insert(p->first);
        }
    }
    return unusedProperties;
}

Ice::PropertiesI::PropertiesI(const PropertiesI* p) :
    _properties(p->_properties),
    _converter(p->_converter)
{
}

Ice::PropertiesI::PropertiesI(const StringConverterPtr& converter) :
    _converter(converter)
{
}

Ice::PropertiesI::PropertiesI(StringSeq& args, const PropertiesPtr& defaults, const StringConverterPtr& converter) :
    _converter(converter)
{
    if(defaults != 0)
    {
        _properties = static_cast<PropertiesI*>(defaults.get())->_properties;
    }

    StringSeq::iterator q = args.begin();

     
    map<string, PropertyValue>::iterator p = _properties.find("Ice.ProgramName");
    if(p == _properties.end())
    {
        if(q != args.end())
        {
            //
            // Use the first argument as the value for Ice.ProgramName. Replace
            // any backslashes in this value with forward slashes, in case this
            // value is used by the event logger.
            //
            string name = *q;
            replace(name.begin(), name.end(), '\\', '/');

            PropertyValue pv(name, true);
            _properties["Ice.ProgramName"] = pv;
        }
    }
    else
    {
        p->second.used = true;
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
            parseLine(s.substr(2), 0);
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
Ice::PropertiesI::parseLine(const string& line, const StringConverterPtr& converter)
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
    
    setProperty(key, value);
}

void
Ice::PropertiesI::loadConfig()
{
    string value = getProperty("Ice.Config");

    if(value.empty() || value == "1")
    {
        const char* s = getenv("ICE_CONFIG");
        if(s && *s != '\0')
        {
            value = s;
        }
    }

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

    PropertyValue pv(value, true);
    _properties["Ice.Config"] = pv;
}
