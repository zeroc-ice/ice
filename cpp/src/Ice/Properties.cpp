// Copyright (c) ZeroC, Inc.

#include "Ice/Properties.h"
#include "FileUtil.h"
#include "Ice/Initialize.h"
#include "Ice/LocalExceptions.h"
#include "Ice/Logger.h"
#include "Ice/LoggerUtil.h"
#include "Ice/StringUtil.h"
#include "PropertyNames.h"
#include "PropertyUtil.h"

#include <algorithm>
#include <cassert>
#include <fstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{
    /// Find the Ice property array for a given property name.
    /// @param key The property name.
    /// @return The property array if found, nullopt otherwise.
    const PropertyArray* findIcePropertyArray(string_view key)
    {
        // Check if the property is legal.
        string::size_type dotPos = key.find('.');

        // If the key doesn't contain a dot, it's not a valid Ice property.
        if (dotPos == string::npos)
        {
            return nullptr;
        }

        string_view prefix = key.substr(0, dotPos);

        // Find the property list for the given prefix.
        for (const auto& properties : IceInternal::PropertyNames::validProps)
        {
            if (prefix == properties.name)
            {
                // We've found the property list for the given prefix.
                return &properties;
            }
        }

        return nullptr;
    }

    /// Find the default value for an Ice property. If there is no default value, return an empty string.
    /// @param key The ice property name.
    /// @return The default value for the property.
    /// @throws PropertyException if the property is unknown.
    string_view getDefaultValue(string_view key)
    {
        auto propertyArray = findIcePropertyArray(key);

        if (!propertyArray)
        {
            throw PropertyException{__FILE__, __LINE__, "unknown Ice property: " + string{key}};
        }

        // The Ice property prefix.
        string prefix{propertyArray->name};

        auto prop = IceInternal::findProperty(key.substr(prefix.length() + 1), propertyArray);

        if (!prop)
        {
            throw PropertyException{__FILE__, __LINE__, "unknown Ice property: " + string{key}};
        }

        return prop->defaultValue;
    }
}

Ice::Properties::Properties(const Properties& p)
{
    lock_guard lock(p._mutex);
    _properties = p._properties;
    _optInPrefixes = p._optInPrefixes;
}

Ice::Properties::Properties(StringSeq& args, const PropertiesPtr& defaults)
{
    if (defaults)
    {
        lock_guard lock(defaults->_mutex);
        _properties = defaults->_properties;
        _optInPrefixes = defaults->_optInPrefixes;
    }

    auto q = args.begin();

    auto p = _properties.find("Ice.ProgramName");
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
            if (auto optionPair = parseLine(s.substr(2), nullptr))
            {
                auto [key, value] = *optionPair;
                setProperty(key, value);
            }
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
Ice::Properties::getProperty(string_view key)
{
    lock_guard lock(_mutex);

    auto p = _properties.find(key);
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
Ice::Properties::getPropertyWithDefault(string_view key, string_view value)
{
    lock_guard lock(_mutex);

    auto p = _properties.find(key);
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
Ice::Properties::getPropertyAsInt(string_view key)
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
Ice::Properties::getPropertyAsIntWithDefault(string_view key, int32_t value)
{
    lock_guard lock(_mutex);

    auto p = _properties.find(key);
    if (p != _properties.end())
    {
        p->second.used = true;
        istringstream v(p->second.value);
        if (!(v >> value) || !v.eof())
        {
            throw PropertyException(
                __FILE__,
                __LINE__,
                "property '" + string{key} + "' has an invalid integer value: '" + p->second.value + "'");
        }
    }

    return value;
}

Ice::StringSeq
Ice::Properties::getPropertyAsList(string_view key)
{
    return getPropertyAsListWithDefault(key, StringSeq());
}

Ice::StringSeq
Ice::Properties::getIcePropertyAsList(string_view key)
{
    string_view defaultValue = getDefaultValue(key);
    StringSeq defaultList;
    IceInternal::splitString(defaultValue, ", \t\r\n", defaultList);
    return getPropertyAsListWithDefault(key, defaultList);
}

Ice::StringSeq
Ice::Properties::getPropertyAsListWithDefault(string_view key, const StringSeq& value)
{
    lock_guard lock(_mutex);

    auto p = _properties.find(key);
    if (p != _properties.end())
    {
        p->second.used = true;

        StringSeq result;
        if (!IceInternal::splitString(p->second.value, ", \t\r\n", result))
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
Ice::Properties::getPropertiesForPrefix(string_view prefix)
{
    lock_guard lock(_mutex);

    PropertyDict result;
    for (auto& prop : _properties)
    {
        if (prefix.empty() || prop.first.compare(0, prefix.size(), prefix) == 0)
        {
            prop.second.used = true;
            result[prop.first] = prop.second.value;
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
    string currentKey = IceInternal::trim(string{key});
    if (currentKey.empty())
    {
        throw InitializationException(__FILE__, __LINE__, "Attempt to set property with empty key");
    }

    // Check if the property is in an Ice property prefix. If so, check that it's a valid property.
    if (auto propertyArray = findIcePropertyArray(key))
    {
        if (propertyArray->isOptIn &&
            find(_optInPrefixes.begin(), _optInPrefixes.end(), propertyArray->name) == _optInPrefixes.end())
        {
            ostringstream os;
            os << "unable to set '" << key << "': property prefix '" << propertyArray->name
               << "' is opt-in and must be explicitly enabled.";
            throw PropertyException{__FILE__, __LINE__, os.str()};
        }
        string propertyPrefix{propertyArray->name};
        auto prop = IceInternal::findProperty(key.substr(propertyPrefix.length() + 1), propertyArray);
        if (!prop)
        {
            throw PropertyException{__FILE__, __LINE__, "unknown Ice property: " + string{key}};
        }

        // If the property is deprecated, log a warning.
        if (prop->deprecated)
        {
            getProcessLogger()->warning("setting deprecated property: " + string{key});
        }
    }

    lock_guard lock(_mutex);

    //
    // Set or clear the property.
    //
    if (!value.empty())
    {
        PropertyValue pv{string{value}, false};
        auto p = _properties.find(currentKey);
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
Ice::Properties::getCommandLineOptions()
{
    lock_guard lock(_mutex);

    StringSeq result;
    result.reserve(_properties.size());
    for (const auto& prop : _properties)
    {
        result.push_back("--" + prop.first + "=" + prop.second.value);
    }
    return result;
}

StringSeq
Ice::Properties::parseCommandLineOptions(string_view prefix, const StringSeq& options)
{
    auto [matched, unmatched] = parseOptions(prefix, options);

    for (const auto& [key, value] : matched)
    {
        setProperty(key, value);
    }

    return unmatched;
}

StringSeq
Ice::Properties::parseIceCommandLineOptions(const StringSeq& options)
{
    StringSeq args = options;
    for (const auto& props : IceInternal::PropertyNames::validProps)
    {
        args = parseCommandLineOptions(props.name, args);
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
                "could not open Windows registry key '" + string{file} + "':\n" + IceInternal::errorToString(err));
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
                    "could not open Windows registry key '" + string{file} + "':\n" + IceInternal::errorToString(err));
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
                    os << "could not read Windows registry property name, key: '" + string{file} + "', index: " << i
                       << ":\n";
                    if (nameBufSize == 0)
                    {
                        os << "property name can't be the empty string";
                    }
                    else
                    {
                        os << IceInternal::errorToString(err);
                    }
                    getProcessLogger()->warning(os.str());
                    continue;
                }
                string name =
                    wstringToString(wstring(reinterpret_cast<wchar_t*>(&nameBuf[0]), nameBufSize), stringConverter);
                if (keyType != REG_SZ && keyType != REG_EXPAND_SZ)
                {
                    ostringstream os;
                    os << "unsupported type for Windows registry property '" + name + "' key: '" + string{file} + "'";
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
                            os << "could not expand variable in property '" << name
                               << "', key: '" + string{file} + "':\n";
                            os << IceInternal::lastErrorToString();
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
        ifstream in(IceInternal::streamFilename(string{file}).c_str());
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
            if (auto optionPair = parseLine(line, stringConverter))
            {
                auto [key, value] = *optionPair;
                setProperty(key, value);
            }
        }
    }
}

set<string>
Ice::Properties::getUnusedProperties()
{
    lock_guard lock(_mutex);
    set<string> unusedProperties;
    for (const auto& prop : _properties)
    {
        if (!prop.second.used)
        {
            unusedProperties.insert(prop.first);
        }
    }
    return unusedProperties;
}

pair<map<string, string>, StringSeq>
Ice::Properties::parseOptions(string_view prefix, const StringSeq& options)
{
    map<string, string> matched;

    string pfx = string{prefix};
    if (!pfx.empty() && pfx[pfx.size() - 1] != '.')
    {
        pfx += '.';
    }
    pfx = "--" + pfx;

    StringSeq unmatched;

    for (auto opt : options)
    {
        if (opt.find(pfx) == 0)
        {
            if (opt.find('=') == string::npos)
            {
                opt += "=1";
            }

            if (auto optionPair = parseLine(opt.substr(2), nullptr))
            {
                matched.insert_or_assign(std::move(optionPair->first), std::move(optionPair->second));
            }
        }
        else
        {
            unmatched.push_back(std::move(opt));
        }
    }

    return {matched, unmatched};
}

optional<pair<string, string>>
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
        return nullopt;
    }
    else if (key.length() == 0)
    {
        return nullopt;
    }

    key = UTF8ToNative(key, converter);
    value = UTF8ToNative(value, converter);

    return make_pair(key, value);
};

void
Ice::Properties::loadConfig()
{
    string value = getIceProperty("Ice.Config");
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
        IceInternal::splitString(value, ",", files);
        for (const auto& file : files)
        {
            load(IceInternal::trim(string{file}));
        }

        PropertyValue pv{std::move(value), true};
        _properties["Ice.Config"] = pv;
    }
}

PropertiesPtr
Ice::createProperties()
{
    return make_shared<Properties>();
}

PropertiesPtr
Ice::createProperties(StringSeq& args, const PropertiesPtr& defaults)
{
    return make_shared<Properties>(args, defaults);
}

PropertiesPtr
Ice::createProperties(int& argc, const char* argv[], const PropertiesPtr& defaults)
{
    StringSeq args = argsToStringSeq(argc, argv);
    PropertiesPtr properties = createProperties(args, defaults);
    stringSeqToArgs(args, argc, argv);
    return properties;
}

#ifdef _WIN32
PropertiesPtr
Ice::createProperties(int& argc, const wchar_t* argv[], const PropertiesPtr& defaults)
{
    StringSeq args = argsToStringSeq(argc, argv);
    PropertiesPtr properties = createProperties(args, defaults);
    stringSeqToArgs(args, argc, argv);
    return properties;
}
#endif
