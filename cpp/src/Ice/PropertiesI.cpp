// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/PropertiesI.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/FileUtil.h>
#include <Ice/Initialize.h>
#include <Ice/LocalException.h>
#include <Ice/PropertyNames.h>
#include <Ice/Logger.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Communicator.h>

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
    for(map<string, PropertyValue>::iterator p = _properties.begin(); p != _properties.end(); ++p)
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

            bool mismatchCase = false;
            string otherKey;
            string propPrefix = pattern.substr(0, dotPos);
            if(IceUtilInternal::toUpper(propPrefix) != IceUtilInternal::toUpper(prefix))
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

                if(!found && IceUtilInternal::match(IceUtilInternal::toUpper(currentKey),
                                                    IceUtilInternal::toUpper(prop.pattern)))
                {
                    found = true;
                    mismatchCase = true;
                    otherKey = prop.pattern;
                    break;
                }
            }
            if(!found)
            {
                logger->warning("unknown property: `" + currentKey + "'");
            }
            else if(mismatchCase)
            {
                logger->warning("unknown property: `" + currentKey + "'; did you mean `" + otherKey + "'");
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
    for(map<string, PropertyValue>::const_iterator p = _properties.begin(); p != _properties.end(); ++p)
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
    for(StringSeq::size_type i = 0; i < options.size(); i++)
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
//
// Metro style applications cannot access Windows registry.
//
#if defined (_WIN32) && !defined(ICE_OS_WINRT)
    if(file.find("HKLM\\") == 0)
    {
        HKEY iceKey;
        const wstring keyName = IceUtil::stringToWstring(file, _converter).substr(5).c_str();
        LONG err;
        if((err = RegOpenKeyExW(HKEY_LOCAL_MACHINE, keyName.c_str(), 0, KEY_QUERY_VALUE, &iceKey)) != ERROR_SUCCESS)
        {
            InitializationException ex(__FILE__, __LINE__);
            ex.reason = "could not open Windows registry key `" + file + "':\n" + IceUtilInternal::errorToString(err);
            throw ex;
        }

        DWORD maxNameSize; // Size in characters not including terminating null character.
        DWORD maxDataSize; // Size in bytes
        DWORD numValues;
        try
        {
            err = RegQueryInfoKey(iceKey, NULL, NULL, NULL, NULL, NULL, NULL, &numValues, &maxNameSize, &maxDataSize,
                                  NULL, NULL);
            if(err != ERROR_SUCCESS)
            {
                InitializationException ex(__FILE__, __LINE__);
                ex.reason = "could not open Windows registry key `" + file + "':\n";
                ex.reason += IceUtilInternal::errorToString(err);
                throw ex;
            }

            for(DWORD i = 0; i < numValues; ++i)
            {
                vector<wchar_t> nameBuf(maxNameSize + 1);
                vector<BYTE> dataBuf(maxDataSize);
                DWORD keyType;
                DWORD nameBufSize = static_cast<DWORD>(nameBuf.size());
                DWORD dataBufSize = static_cast<DWORD>(dataBuf.size());
                err = RegEnumValueW(iceKey, i, &nameBuf[0], &nameBufSize, NULL, &keyType, &dataBuf[0], &dataBufSize);
                if(err != ERROR_SUCCESS || nameBufSize == 0)
                {
                    ostringstream os;
                    os << "could not read Windows registry property name, key: `" + file + "', index: " << i << ":\n";
                    if(nameBufSize == 0)
                    {
                        os << "property name can't be the empty string";
                    }
                    else
                    {
                        os << IceUtilInternal::errorToString(err);
                    }
                    getProcessLogger()->warning(os.str());
                    continue;
                }
                string name = IceUtil::wstringToString(
                    wstring(reinterpret_cast<wchar_t*>(&nameBuf[0]), nameBufSize), _converter);
                if(keyType != REG_SZ && keyType != REG_EXPAND_SZ)
                {
                    ostringstream os;
                    os << "unsupported type for Windows registry property `" + name + "' key: `" + file + "'";
                    getProcessLogger()->warning(os.str());
                    continue;
                }

                string value;
                wstring valueW = wstring(reinterpret_cast<wchar_t*>(&dataBuf[0]), (dataBufSize / sizeof(wchar_t)) - 1);
                if(keyType == REG_SZ)
                {
                    value = IceUtil::wstringToString(valueW, _converter);
                }
                else // keyType == REG_EXPAND_SZ
                {
                    vector<wchar_t> expandedValue(1024);
                    DWORD sz = ExpandEnvironmentStringsW(valueW.c_str(), &expandedValue[0],
                                                         static_cast<DWORD>(expandedValue.size()));
                    if(sz >= expandedValue.size())
                    {
                        expandedValue.resize(sz + 1);
                        if(ExpandEnvironmentStringsW(valueW.c_str(), &expandedValue[0],
                                                     static_cast<DWORD>(expandedValue.size())) == 0)
                        {
                            ostringstream os;
                            os << "could not expand variable in property `" << name << "', key: `" + file + "':\n";
                            os << IceUtilInternal::lastErrorToString();
                            getProcessLogger()->warning(os.str());
                            continue;
                        }
                    }
                    value = IceUtil::wstringToString(wstring(&expandedValue[0], sz -1), _converter);
                }
                setProperty(name, value);
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
        IceUtilInternal::ifstream in(file);
        if(!in)
        {
            FileException ex(__FILE__, __LINE__);
            ex.path = file;
            ex.error = getSystemErrno();
            throw ex;
        }

        string line;
        bool firstLine = true;
        while(getline(in, line))
        {
            //
            // Skip UTF8 BOM if present.
            //
            if(firstLine)
            {
                const unsigned char UTF8_BOM[3] = {0xEF, 0xBB, 0xBF};
                if(line.size() >= 3 &&
                   static_cast<const unsigned char>(line[0]) == UTF8_BOM[0] &&
                   static_cast<const unsigned char>(line[1]) == UTF8_BOM[1] &&
                   static_cast<const unsigned char>(line[2]) == UTF8_BOM[2])
                {
                    line = line.substr(3);
                }
                firstLine = false;
            }
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

Ice::PropertiesI::PropertiesI(const IceUtil::StringConverterPtr& converter) :
    _converter(converter)
{
}

Ice::PropertiesI::PropertiesI(StringSeq& args, const PropertiesPtr& defaults, const IceUtil::StringConverterPtr& converter) :
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
Ice::PropertiesI::parseLine(const string& line, const IceUtil::StringConverterPtr& converter)
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

    key = IceUtil::UTF8ToNative(key, converter);
    value = IceUtil::UTF8ToNative(value, converter);

    setProperty(key, value);
}

void
Ice::PropertiesI::loadConfig()
{
    string value = getProperty("Ice.Config");
#ifndef ICE_OS_WINRT
    //
    // WinRT cannot access environment variables
    //
    if(value.empty() || value == "1")
    {
#   ifdef _WIN32
        vector<wchar_t> v(256);
        DWORD ret = GetEnvironmentVariableW(L"ICE_CONFIG", &v[0], static_cast<DWORD>(v.size()));
        if(ret >= v.size())
        {
            v.resize(ret + 1);
            ret = GetEnvironmentVariableW(L"ICE_CONFIG", &v[0], static_cast<DWORD>(v.size()));
        }
        if(ret > 0)
        {
            value = IceUtil::wstringToString(wstring(&v[0], ret), _converter);
        }
        else
        {
            value = "";
        }
#   else
       const char* s = getenv("ICE_CONFIG");
       if(s && *s != '\0')
       {
           value = s;
       }
#   endif
    }
#endif

    if(!value.empty())
    {
        vector<string> files;
        IceUtilInternal::splitString(value, ",", files);
        for(vector<string>::const_iterator i = files.begin(); i != files.end(); ++i)
        {
            load(IceUtilInternal::trim(*i));
        }

        PropertyValue pv(value, true);
        _properties["Ice.Config"] = pv;
    }
}
