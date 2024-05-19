//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/Properties.h"
#include "Ice/Initialize.h"
#include "Ice/LocalException.h"
#include "Ice/Logger.h"
#include "Ice/LoggerUtil.h"
#include "IceUtil/FileUtil.h"
#include "IceUtil/StringUtil.h"
#include "PropertyNames.h"

#include <cassert>
#include <fstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{
    /// Find a property in the Ice property set.
    /// @param key The property name.
    /// @param logWarnings Whether to log relevant warnings.
    /// @return The property if found, nullopt otherwise.
    optional<Property> findProperty(string_view key, bool logWarnings)
    {
        // Check if the property is legal.
        LoggerPtr logger = getProcessLogger();
        string::size_type dotPos = key.find('.');

        // If the key doesn't contain a dot, it's not a valid Ice property.
        if (dotPos == string::npos)
        {
            return nullopt;
        }

        string_view prefix = key.substr(0, dotPos);

        // Find the property list for the given prefix.
        const IceInternal::PropertyArray* propertyArray = nullptr;

        for (int i = 0; IceInternal::PropertyNames::validProps[i].properties != nullptr; ++i)
        {
            string_view pattern{IceInternal::PropertyNames::validProps[i].properties[0].pattern};
            dotPos = pattern.find('.');

            // Each top level prefix describes a non-empty
            // namespace. Having a string without a prefix followed by a
            // dot is an error.
            assert(dotPos != string::npos);
            string_view propPrefix = pattern.substr(0, dotPos);

            if (propPrefix == prefix)
            {
                // We've found the property list for the given prefix.
                propertyArray = &IceInternal::PropertyNames::validProps[i];
                break;
            }

            // As a courtesy to the user, perform a case-insensitive match and suggest the correct property.
            // Otherwise no other warning is issued.
            if (logWarnings && IceUtilInternal::toLower(propPrefix) == IceUtilInternal::toLower(prefix))
            {
                ostringstream os;
                os << "unknown property prefix: `" << prefix << "'; did you mean `" << propPrefix << "'?";
                return nullopt;
            }
        }

        if (!propertyArray)
        {
            // The prefix is not a valid Ice property.
            return nullopt;
        }

        for (int i = 0; i < propertyArray->length; ++i)
        {
            auto prop = propertyArray->properties[i];

            if (prop.usesRegex ? IceUtilInternal::match(string{key}, prop.pattern) : key == prop.pattern)
            {
                return prop;
            }
        }

        if (logWarnings)
        {
            ostringstream os;
            os << "unknown property: `" << key << "'";
            logger->warning(os.str());
        }
        return nullopt;
    }

    /// Find the default value for an Ice property. If there is no default value, return an empty string.
    /// @param key The ice property name.
    /// @return The default value for the property.
    /// @throws std::invalid_argument if the property is unknown.
    string_view getDefaultValue(string_view key)
    {
        optional<Property> prop = findProperty(key, false);
        if (!prop)
        {
            throw invalid_argument{"unknown Ice property: " + string{key}};
        }
        return prop->defaultValue;
    }
}

Ice::Properties::Properties(const Properties& p)
{
    lock_guard lock(p._mutex);
    _properties = p._properties;
}

Ice::Properties::Properties(StringSeq& args, const PropertiesPtr& defaults)
{
    if (defaults)
    {
        lock_guard lock(defaults->_mutex);
        _properties = defaults->_properties;
    }

    StringSeq::iterator q = args.begin();

    map<string, PropertyValue>::iterator p = _properties.find("Ice.ProgramName");
    if (p == _properties.end())
    {
        if (q != args.end())
        {
            //
            // Use the first argument as the value for Ice.ProgramName. Replace
            // any backslashes in this value with forward slashes, in case this
            // value is used by the event logger.
            //
            string name = *q;
            replace(name.begin(), name.end(), '\\', '/');

            PropertyValue pv{std::move(name), true};
            _properties["Ice.ProgramName"] = pv;
        }
    }
    else
    {
        p->second.used = true;
    }

    StringSeq tmp;

    bool loadConfigFiles = false;
    while (q != args.end())
    {
        string s = *q;
        if (s.find("--Ice.Config") == 0)
        {
            if (s.find('=') == string::npos)
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

    if (!loadConfigFiles)
    {
        //
        // If Ice.Config is not set, load from ICE_CONFIG (if set)
        //
        loadConfigFiles = (_properties.find("Ice.Config") == _properties.end());
    }

    if (loadConfigFiles)
    {
        loadConfig();
    }

    args = parseIceCommandLineOptions(args);
}

string
Ice::Properties::getProperty(string_view key) noexcept
{
    lock_guard lock(_mutex);

    map<string, PropertyValue>::iterator p = _properties.find(key);
    if (p != _properties.end())
    {
        p->second.used = true;
        return p->second.value;
    }
    else
    {
        return string{};
    }
}

string
Ice::Properties::getIceProperty(string_view key)
{
    string_view defaultValue = getDefaultValue(key);
    return getPropertyWithDefault(key, defaultValue);
}

string
Ice::Properties::getPropertyWithDefault(string_view key, string_view value) noexcept
{
    lock_guard lock(_mutex);

    map<string, PropertyValue>::iterator p = _properties.find(key);
    if (p != _properties.end())
    {
        p->second.used = true;
        return p->second.value;
    }
    else
    {
        return string{value};
    }
}

int32_t
Ice::Properties::getPropertyAsInt(string_view key) noexcept
{
    return getPropertyAsIntWithDefault(key, 0);
}

int32_t
Ice::Properties::getIcePropertyAsInt(string_view key)
{
    string defaultValueString{getDefaultValue(key)};
    int32_t defaultValue{0};

    if (!defaultValueString.empty())
    {
        // If the default value is not empty, it should be a number.
        // These come from the IceInternal::PropertyNames::validProps array so they should be valid.
        defaultValue = stoi(defaultValueString);
    }

    return getPropertyAsIntWithDefault(key, defaultValue);
}

int32_t
Ice::Properties::getPropertyAsIntWithDefault(string_view key, int32_t value) noexcept
{
    lock_guard lock(_mutex);

    map<string, PropertyValue>::iterator p = _properties.find(key);
    if (p != _properties.end())
    {
        int32_t val = value;
        p->second.used = true;
        istringstream v(p->second.value);
        if (!(v >> value) || !v.eof())
        {
            Warning out(getProcessLogger());
            out << "numeric property " << key << " set to non-numeric value, defaulting to " << val;
            return val;
        }
    }

    return value;
}

Ice::StringSeq
Ice::Properties::getPropertyAsList(string_view key) noexcept
{
    return getPropertyAsListWithDefault(key, StringSeq());
}

Ice::StringSeq
Ice::Properties::getIcePropertyAsList(string_view key)
{
    string_view defaultValue = getDefaultValue(key);
    StringSeq defaultList;
    IceUtilInternal::splitString(defaultValue, ", \t\r\n", defaultList);
    return getPropertyAsListWithDefault(key, defaultList);
}

Ice::StringSeq
Ice::Properties::getPropertyAsListWithDefault(string_view key, const StringSeq& value) noexcept
{
    lock_guard lock(_mutex);

    map<string, PropertyValue>::iterator p = _properties.find(key);
    if (p != _properties.end())
    {
        p->second.used = true;

        StringSeq result;
        if (!IceUtilInternal::splitString(p->second.value, ", \t\r\n", result))
        {
            Warning out(getProcessLogger());
            out << "mismatched quotes in property " << key << "'s value, returning default value";
        }
        if (result.size() == 0)
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
Ice::Properties::getPropertiesForPrefix(string_view prefix) noexcept
{
    lock_guard lock(_mutex);

    PropertyDict result;
    for (map<string, PropertyValue>::iterator p = _properties.begin(); p != _properties.end(); ++p)
    {
        if (prefix.empty() || p->first.compare(0, prefix.size(), prefix) == 0)
        {
            p->second.used = true;
            result[p->first] = p->second.value;
        }
    }

    return result;
}

void
Ice::Properties::setProperty(string_view key, string_view value)
{
    //
    // Trim whitespace
    //
    string currentKey = IceUtilInternal::trim(string{key});
    if (currentKey.empty())
    {
        throw InitializationException(__FILE__, __LINE__, "Attempt to set property with empty key");
    }

    // Finds the corresponding Ice property if it exists. Also logs warnings for unknown Ice properties and
    // case-insensitive Ice property prefix matches.
    auto prop = findProperty(key, true);

    // If the property is deprecated, log a warning.
    if (prop && prop->deprecated)
    {
        getProcessLogger()->warning("setting deprecated property: " + string{key});
    }

    lock_guard lock(_mutex);

    //
    // Set or clear the property.
    //
    if (!value.empty())
    {
        PropertyValue pv{string{value}, false};
        map<string, PropertyValue>::const_iterator p = _properties.find(currentKey);
        if (p != _properties.end())
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
Ice::Properties::getCommandLineOptions() noexcept
{
    lock_guard lock(_mutex);

    StringSeq result;
    result.reserve(_properties.size());
    for (map<string, PropertyValue>::const_iterator p = _properties.begin(); p != _properties.end(); ++p)
    {
        result.push_back("--" + p->first + "=" + p->second.value);
    }
    return result;
}

StringSeq
Ice::Properties::parseCommandLineOptions(string_view prefix, const StringSeq& options)
{
    string pfx = string{prefix};
    if (!pfx.empty() && pfx[pfx.size() - 1] != '.')
    {
        pfx += '.';
    }
    pfx = "--" + pfx;

    StringSeq result;
    for (StringSeq::size_type i = 0; i < options.size(); i++)
    {
        string opt = options[i];

        if (opt.find(pfx) == 0)
        {
            if (opt.find('=') == string::npos)
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
Ice::Properties::parseIceCommandLineOptions(const StringSeq& options)
{
    StringSeq args = options;
    for (const char** i = IceInternal::PropertyNames::clPropNames; *i != 0; ++i)
    {
        args = parseCommandLineOptions(*i, args);
    }
    return args;
}

void
Ice::Properties::load(string_view file)
{
    StringConverterPtr stringConverter = getProcessStringConverter();

#if defined(_WIN32)
    if (file.find("HKCU\\") == 0 || file.find("HKLM\\") == 0)
    {
        HKEY key = file.find("HKCU\\") == 0 ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
        HKEY iceKey;
        const wstring keyName = stringToWstring(string{file}, stringConverter).substr(file.find("\\") + 1).c_str();
        LONG err;
        if ((err = RegOpenKeyExW(key, keyName.c_str(), 0, KEY_QUERY_VALUE, &iceKey)) != ERROR_SUCCESS)
        {
            throw InitializationException(
                __FILE__,
                __LINE__,
                "could not open Windows registry key `" + string{file} + "':\n" + IceUtilInternal::errorToString(err));
        }

        DWORD maxNameSize; // Size in characters not including terminating null character.
        DWORD maxDataSize; // Size in bytes
        DWORD numValues;
        try
        {
            err = RegQueryInfoKey(
                iceKey,
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                &numValues,
                &maxNameSize,
                &maxDataSize,
                nullptr,
                nullptr);
            if (err != ERROR_SUCCESS)
            {
                throw InitializationException(
                    __FILE__,
                    __LINE__,
                    "could not open Windows registry key `" + string{file} + "':\n" +
                        IceUtilInternal::errorToString(err));
            }

            for (DWORD i = 0; i < numValues; ++i)
            {
                vector<wchar_t> nameBuf(maxNameSize + 1);
                vector<BYTE> dataBuf(maxDataSize);
                DWORD keyType;
                DWORD nameBufSize = static_cast<DWORD>(nameBuf.size());
                DWORD dataBufSize = static_cast<DWORD>(dataBuf.size());
                err = RegEnumValueW(iceKey, i, &nameBuf[0], &nameBufSize, nullptr, &keyType, &dataBuf[0], &dataBufSize);
                if (err != ERROR_SUCCESS || nameBufSize == 0)
                {
                    ostringstream os;
                    os << "could not read Windows registry property name, key: `" + string{file} + "', index: " << i
                       << ":\n";
                    if (nameBufSize == 0)
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
                string name =
                    wstringToString(wstring(reinterpret_cast<wchar_t*>(&nameBuf[0]), nameBufSize), stringConverter);
                if (keyType != REG_SZ && keyType != REG_EXPAND_SZ)
                {
                    ostringstream os;
                    os << "unsupported type for Windows registry property `" + name + "' key: `" + string{file} + "'";
                    getProcessLogger()->warning(os.str());
                    continue;
                }

                string value;
                wstring valueW = wstring(reinterpret_cast<wchar_t*>(&dataBuf[0]), (dataBufSize / sizeof(wchar_t)) - 1);
                if (keyType == REG_SZ)
                {
                    value = wstringToString(valueW, stringConverter);
                }
                else // keyType == REG_EXPAND_SZ
                {
                    vector<wchar_t> expandedValue(1024);
                    DWORD sz = ExpandEnvironmentStringsW(
                        valueW.c_str(),
                        &expandedValue[0],
                        static_cast<DWORD>(expandedValue.size()));
                    if (sz >= expandedValue.size())
                    {
                        expandedValue.resize(sz + 1);
                        if (ExpandEnvironmentStringsW(
                                valueW.c_str(),
                                &expandedValue[0],
                                static_cast<DWORD>(expandedValue.size())) == 0)
                        {
                            ostringstream os;
                            os << "could not expand variable in property `" << name
                               << "', key: `" + string{file} + "':\n";
                            os << IceUtilInternal::lastErrorToString();
                            getProcessLogger()->warning(os.str());
                            continue;
                        }
                    }
                    value = wstringToString(wstring(&expandedValue[0], sz - 1), stringConverter);
                }
                setProperty(name, value);
            }
        }
        catch (...)
        {
            RegCloseKey(iceKey);
            throw;
        }
        RegCloseKey(iceKey);
    }
    else
#endif
    {
        ifstream in(IceUtilInternal::streamFilename(string{file}).c_str());
        if (!in)
        {
            throw FileException(__FILE__, __LINE__, string{file});
        }

        string line;
        bool firstLine = true;
        while (getline(in, line))
        {
            //
            // Skip UTF8 BOM if present.
            //
            if (firstLine)
            {
                const unsigned char UTF8_BOM[3] = {0xEF, 0xBB, 0xBF};
                if (line.size() >= 3 && static_cast<unsigned char>(line[0]) == UTF8_BOM[0] &&
                    static_cast<unsigned char>(line[1]) == UTF8_BOM[1] &&
                    static_cast<unsigned char>(line[2]) == UTF8_BOM[2])
                {
                    line = line.substr(3);
                }
                firstLine = false;
            }
            parseLine(line, stringConverter);
        }
    }
}

set<string>
Ice::Properties::getUnusedProperties()
{
    lock_guard lock(_mutex);
    set<string> unusedProperties;
    for (map<string, PropertyValue>::const_iterator p = _properties.begin(); p != _properties.end(); ++p)
    {
        if (!p->second.used)
        {
            unusedProperties.insert(p->first);
        }
    }
    return unusedProperties;
}

void
Ice::Properties::parseLine(string_view line, const StringConverterPtr& converter)
{
    string key;
    string value;

    enum ParseState
    {
        Key,
        Value
    };
    ParseState state = Key;

    string whitespace;
    string escapedspace;
    bool finished = false;
    for (string::size_type i = 0; i < line.size(); ++i)
    {
        char c = line[i];
        switch (state)
        {
            case Key:
            {
                switch (c)
                {
                    case '\\':
                        if (i < line.length() - 1)
                        {
                            c = line[++i];
                            switch (c)
                            {
                                case '\\':
                                case '#':
                                case '=':
                                    key += whitespace;
                                    whitespace.clear();
                                    key += c;
                                    break;

                                case ' ':
                                    if (key.length() != 0)
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
                        if (key.length() != 0)
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
                switch (c)
                {
                    case '\\':
                        if (i < line.length() - 1)
                        {
                            c = line[++i];
                            switch (c)
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
                        if (value.length() != 0)
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
        if (finished)
        {
            break;
        }
    }
    value += escapedspace;

    if ((state == Key && key.length() != 0) || (state == Value && key.length() == 0))
    {
        getProcessLogger()->warning("invalid config file entry: \"" + string{line} + "\"");
        return;
    }
    else if (key.length() == 0)
    {
        return;
    }

    key = UTF8ToNative(key, converter);
    value = UTF8ToNative(value, converter);

    setProperty(key, value);
}

void
Ice::Properties::loadConfig()
{
    string value = getProperty("Ice.Config");
    if (value.empty() || value == "1")
    {
#ifdef _WIN32
        vector<wchar_t> v(256);
        DWORD ret = GetEnvironmentVariableW(L"ICE_CONFIG", &v[0], static_cast<DWORD>(v.size()));
        if (ret >= v.size())
        {
            v.resize(ret + 1);
            ret = GetEnvironmentVariableW(L"ICE_CONFIG", &v[0], static_cast<DWORD>(v.size()));
        }
        if (ret > 0)
        {
            value = wstringToString(wstring(&v[0], ret), getProcessStringConverter());
        }
        else
        {
            value = "";
        }
#else
        const char* s = getenv("ICE_CONFIG");
        if (s && *s != '\0')
        {
            value = s;
        }
#endif
    }

    if (!value.empty())
    {
        vector<string> files;
        IceUtilInternal::splitString(value, ",", files);
        for (vector<string>::const_iterator i = files.begin(); i != files.end(); ++i)
        {
            load(IceUtilInternal::trim(string{*i}));
        }

        PropertyValue pv{std::move(value), true};
        _properties["Ice.Config"] = pv;
    }
}
