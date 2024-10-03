// Copyright (c) ZeroC, Inc.

#include "PropertyUtil.h"
#include "Ice/LocalExceptions.h"
#include "Ice/Properties.h"
#include "Ice/StringUtil.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

optional<Property>
IceInternal::findProperty(string_view key, const PropertyArray* propertyArray)
{
    for (int i = 0; i < propertyArray->length; ++i)
    {
        auto prop = propertyArray->properties[i];

        // If the key is an exact match, return the property if it's not a property class. If it is, return nullopt.
        // If the key is a regex match, return the property. A property cannot have a property class and use regex.
        if (key == prop.pattern)
        {
            if (prop.propertyClass && prop.prefixOnly)
            {
                return nullopt;
            }
            return prop;
        }
        else if (prop.usesRegex && match(string{key}, prop.pattern))
        {
            return prop;
        }

        // If the property has a property class, check if the key is a prefix of the property.
        if (prop.propertyClass)
        {
            auto pattern = string{prop.pattern};
            // Check if the key is a prefix of the property.
            // The key must be:
            // - shorter than the property pattern
            // - the property pattern must start with the key
            // - the pattern character after the key must be a dot
            if (key.length() > pattern.length() && key.find(pattern) == 0 && key[pattern.length()] == '.')
            {
                // Plus one to skip the dot.
                string_view substring = key.substr(pattern.length() + 1);
                // Check if the suffix is a valid property. If so, return it. If it's not, continue searching
                // the current property array.
                if (auto subProp = findProperty(substring, prop.propertyClass))
                {
                    return subProp;
                }
            }
        }
    }

    return nullopt;
}

void
IceInternal::validatePropertiesWithPrefix(
    string_view prefix,
    const PropertiesPtr& properties,
    const PropertyArray* propertyArray)
{
    vector<string> unknownProps;
    PropertyDict props = properties->getPropertiesForPrefix(string{prefix} + ".");

    // Do not check for unknown properties if Ice prefix, ie Ice, Glacier2, etc
    for (const char** i = IceInternal::PropertyNames::clPropNames; *i; ++i)
    {
        string icePrefix = string(*i) + ".";
        if (prefix.find(icePrefix) == 0)
        {
            return;
        }
    }

    for (const auto& p : props)
    {
        // Plus one to include the dot.
        if (!findProperty(p.first.substr(prefix.size() + 1), propertyArray))
        {
            unknownProps.push_back(p.first);
        }
    }

    if (!unknownProps.empty())
    {
        ostringstream os;
        os << "found unknown properties for " << propertyArray->name << ": '" << prefix << "'";
        if (!unknownProps.empty())
        {
            for (const auto& prop : unknownProps)
            {
                os << "\n    " << prop;
            }
        }

        throw UnknownPropertyException(__FILE__, __LINE__, os.str());
    }
}
