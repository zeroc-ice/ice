// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
#include <Ice/LoggerUtil.h>
#include <Ice/Communicator.h>
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
        Int val = value;
        p->second.used = true;
        istringstream v(p->second.value);
        if(!(v >> value) || !v.eof())
        {
            Warning out(getProcessLogger());
            out << "numeric property " << key << " set to non-numeric value, defaulting to " << val;
            return val;
        }
    }

    return value;
}

Ice::StringSeq
Ice::PropertiesI::getPropertyAsList(const string& key)
{
    return getPropertyAsListWithDefault(key, StringSeq());
}

Ice::StringSeq
Ice::PropertiesI::getPropertyAsListWithDefault(const string& key, const StringSeq& value)
{
    IceUtil::Mutex::Lock sync(*this);
    
    map<string, PropertyValue>::iterator p = _properties.find(key);
    if(p != _properties.end())
    {
        p->second.used = true;

        StringSeq result;
        if(!IceUtilInternal::splitString(p->second.value, ", \t\r\n", result))
        {
            Warning out(getProcessLogger());
            out << "mismatched quotes in property " << key << "'s value, returning default value";
        }
        if(result.size() == 0)
        {
            result = value;
        }
        return result;
    }
    else
    {
        return value;
    }
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
    //
    // Trim whitespace
    //
    string currentKey = IceUtilInternal::trim(key);
    if(currentKey.empty())
    {
        throw InitializationException(__FILE__, __LINE__, "Attempt to set property with empty key");
    }

    //
    // Check if the property is legal.
    //
    LoggerPtr logger = getProcessLogger();
    string::size_type dotPos = currentKey.find('.');
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
                found = IceUtilInternal::match(currentKey, prop.pattern);

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
#ifdef _WIN32
    if(file.find("HKLM\\") == 0 || file.find("HKCU\\") == 0)
    {
        HKEY key;
        if(file.find("HKLM\\") == 0)
        {
            key = HKEY_LOCAL_MACHINE;
        }
        else
        {
            key = HKEY_CURRENT_USER;
        }

        HKEY iceKey;
        if(RegOpenKeyExW(key, IceUtil::stringToWstring(file.substr(5)).c_str(), 0, KEY_QUERY_VALUE, &iceKey) != ERROR_SUCCESS)
        {
            InitializationException ex(__FILE__, __LINE__);
            ex.reason = "Could not open Windows registry key `" + file + "'";
            throw ex;
        }

        DWORD maxValueNameLen;
        DWORD maxValueLen;
        DWORD numValues;
        try
        {
            if(RegQueryInfoKey(iceKey, NULL, NULL, NULL, NULL, NULL, NULL, &numValues, &maxValueNameLen, &maxValueLen,
                               NULL, NULL) == ERROR_SUCCESS && numValues > 0)
            {
                auto_ptr<wchar_t> keyBuf(new wchar_t[maxValueNameLen + 1]);
                auto_ptr<wchar_t> valueBuf(new wchar_t[maxValueLen + 1]);
                for(DWORD i = 0; i < numValues; ++i)
                {
                    DWORD keyBufSize = (maxValueNameLen + 1) * sizeof(wchar_t);
                    DWORD valueBufSize = (maxValueLen + 1) * sizeof(wchar_t) ;
                    unsigned int err = RegEnumValueW(iceKey, i, keyBuf.get(), &keyBufSize, NULL, NULL, NULL, NULL);
                    if(err != ERROR_SUCCESS)
                    {
                        getProcessLogger()->warning("Could not read Windows registry property name, key path: \"" +
                                                    file + "\"");
                        continue;
                    }
                    keyBuf.get()[keyBufSize] = L'\0';

                    DWORD keyType;
                    err = RegQueryValueExW(iceKey, keyBuf.get(), 0, &keyType, (BYTE*)valueBuf.get(), &valueBufSize);
                    if(err != ERROR_SUCCESS)
                    {
                        getProcessLogger()->warning("Could not read Windows registry property value, property name: `" +
                                                    IceUtil::wstringToString(wstring(keyBuf.get())) + "' key path: `" +
                                                    file + "'");
                        continue;
                    }
                    valueBuf.get()[valueBufSize] = L'\0';

                    switch(keyType)
                    {
                        case REG_SZ:
                        {
                            string name = IceUtil::wstringToString(wstring(keyBuf.get()));
                            string value = IceUtil::wstringToString(wstring(valueBuf.get()));
                            if(_converter)
                            {
                                string tmp;
                                _converter->fromUTF8(reinterpret_cast<const Byte*>(name.data()),
                                                    reinterpret_cast<const Byte*>(name.data() + name.size()), tmp);
                                name.swap(tmp);

                                _converter->fromUTF8(reinterpret_cast<const Byte*>(value.data()),
                                                    reinterpret_cast<const Byte*>(value.data() + value.size()), tmp);
                                value.swap(tmp);
                            }
                            setProperty(name, value);
                            break;
                        }
                        case REG_EXPAND_SZ:
                        {
                            unsigned int sz = ExpandEnvironmentStringsW(valueBuf.get(), 0, 0);
                            auto_ptr<wchar_t> expandValue;
                            if(sz > 0)
                            {
                                valueBufSize = sz;
                                expandValue = auto_ptr<wchar_t>(new wchar_t[sz + 1]);
                                sz = ExpandEnvironmentStringsW(valueBuf.get(), expandValue.get(), sz);
                            }

                            if(sz == 0 || sz > valueBufSize || expandValue.get() == 0)
                            {
                                getProcessLogger()->warning("Could not expand variables in property value: `" + 
                                                            IceUtil::wstringToString(wstring(valueBuf.get())) + 
                                                            "' key path: `" + file + "'");
                                continue;
                            }

                            string name = IceUtil::wstringToString(wstring(keyBuf.get()));
                            string value = IceUtil::wstringToString(wstring(expandValue.get()));
                            if(_converter)
                            {
                                string tmp;
                                _converter->fromUTF8(reinterpret_cast<const Byte*>(name.data()),
                                                    reinterpret_cast<const Byte*>(name.data() + name.size()), tmp);
                                name.swap(tmp);

                                _converter->fromUTF8(reinterpret_cast<const Byte*>(value.data()),
                                                    reinterpret_cast<const Byte*>(value.data() + value.size()), tmp);
                                value.swap(tmp);
                            }
                            setProperty(name, value);
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }
                }
            }
        }
        catch(...)
        {
            RegCloseKey(iceKey);
            throw;
        }
        RegCloseKey(iceKey);
    }
    else
#endif
    {
        ifstream in(file.c_str());
        if(!in)
        {
            FileException ex(__FILE__, __LINE__);
            ex.path = file;
            ex.error = getSystemErrno();
            throw ex;
        }

        string line;
        while(getline(in, line))
        {
            parseLine(line, _converter);
        }
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
    string key;
    string value;
    
    enum ParseState { Key , Value };
    ParseState state = Key;

    string whitespace;
    string escapedspace;
    bool finished = false;
    for(string::size_type i = 0; i < line.size(); ++i)
    {
        char c = line[i];
        switch(state)
        {
          case Key:
          {
            switch(c)
            {
              case '\\':
                if(i < line.length() - 1)
                {
                    c = line[++i];
                    switch(c)
                    {
                      case '\\':
                      case '#':
                      case '=':
                        key += whitespace;
                        whitespace.clear();
                        key += c; 
                        break;

                      case ' ':
                        if(key.length() != 0)
                        {
                            whitespace += c;
                        }
                        break;

                      default:
                        key += whitespace;
                        whitespace.clear();
                        key += '\\';
                        key += c;
                        break;
                    }
                }
                else
                {
                    key += whitespace;
                    key += c;
                }
                break;

              case ' ':
              case '\t':
              case '\r':
              case '\n':
                  if(key.length() != 0)
                  {
                      whitespace += c;
                  }
                  break;

              case '=':
                  whitespace.clear();
                  state = Value;
                  break;

              case '#':
                  finished = true;
                  break;
              
              default:
                  key += whitespace;
                  whitespace.clear();
                  key += c;
                  break;
            }
            break;
          }

          case Value:
          {
            switch(c)
            {
              case '\\':
                if(i < line.length() - 1)
                {
                    c = line[++i];
                    switch(c)
                    {
                      case '\\':
                      case '#':
                      case '=':
                        value += value.length() == 0 ? escapedspace : whitespace;
                        whitespace.clear();
                        escapedspace.clear();
                        value += c; 
                        break;

                      case ' ':
                        whitespace += c;
                        escapedspace += c;
                        break;

                      default:
                        value += value.length() == 0 ? escapedspace : whitespace;
                        whitespace.clear();
                        escapedspace.clear();
                        value += '\\';
                        value += c;
                        break;
                    }
                }
                else
                {
                    value += value.length() == 0 ? escapedspace : whitespace;
                      value += c;
                }
                break;

              case ' ':
              case '\t':
              case '\r':
              case '\n':
                  if(value.length() != 0)
                  {
                      whitespace += c;
                  }
                  break;

              case '#':
                  value += escapedspace;
                  finished = true;
                  break;
              
              default:
                  value += value.length() == 0 ? escapedspace : whitespace;
                  whitespace.clear();
                  escapedspace.clear();
                  value += c;
                  break;
            }
            break;
          }
        }
        if(finished)
        {
            break;
        }
    }
    value += escapedspace;

    if((state == Key && key.length() != 0) || (state == Value && key.length() == 0))
    {
        getProcessLogger()->warning("invalid config file entry: \"" + line + "\"");
        return;
    }
    else if(key.length() == 0)
    {
        return;
    }

    if(converter)
    {
        string tmp;
        converter->fromUTF8(reinterpret_cast<const Byte*>(key.data()),
                            reinterpret_cast<const Byte*>(key.data() + key.size()), tmp);
        key.swap(tmp);

        if(!value.empty())
        {
            converter->fromUTF8(reinterpret_cast<const Byte*>(value.data()),
                                reinterpret_cast<const Byte*>(value.data() + value.size()), tmp);
            value.swap(tmp);
        }
    }
    
    setProperty(key, value);
}

void
Ice::PropertiesI::loadConfig()
{
    string value = getProperty("Ice.Config");

    if(value.empty() || value == "1")
    {
 #ifdef _WIN32
        DWORD ret = GetEnvironmentVariableW(L"ICE_CONFIG", 0, 0);
        if(ret > 0)
        {
            auto_ptr<wchar_t> v(new wchar_t[ret]);
            ret = GetEnvironmentVariableW(L"ICE_CONFIG", v.get(), ret);
            value = (ret > 0 && ret < sizeof(v.get()) / sizeof(wchar_t)) ? IceUtil::wstringToString(v.get()) : string("");
            if(_converter)
            {
                string tmp;
                _converter->fromUTF8(reinterpret_cast<const Byte*>(value.data()),
                                        reinterpret_cast<const Byte*>(value.data() + value.size()), tmp);
                value.swap(tmp);
            }
        }
#else
       const char* s = getenv("ICE_CONFIG");
       if(s && *s != '\0')
       {
           value = s;
       }
#endif
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


//
// PropertiesAdminI
//


Ice::PropertiesAdminI::PropertiesAdminI(const PropertiesPtr& properties) :
    _properties(properties)
{    
}

string 
Ice::PropertiesAdminI::getProperty(const string& name, const Ice::Current&)
{
    return _properties->getProperty(name);
}

Ice::PropertyDict 
Ice::PropertiesAdminI::getPropertiesForPrefix(const string& prefix, const Ice::Current&)
{
    return _properties->getPropertiesForPrefix(prefix);
}
