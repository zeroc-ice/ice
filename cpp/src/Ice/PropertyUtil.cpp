// Copyright (c) ZeroC, Inc.

#include "PropertyUtil.h"
#include "Ice/LocalExceptions.h"
#include "Ice/Properties.h"
#include "Ice/StringUtil.h"

#include <sstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

optional<Property>
IceInternal::findProperty(string_view key, const PropertyArray* propertyArray)
{
    assert(propertyArray);
    assert(propertyArray->properties != nullptr || propertyArray->length == 0);

    for (int i = 0; i < propertyArray->length; ++i)
    {
        auto prop = propertyArray->properties[i];

        // If the key is an exact match, return the property unless it has a property class which is prefix only.
        // If the key is a regex match, return the property. A property cannot have a property class and use regex.
        if (key == prop.pattern)
        {
            if (prop.propertyArray && prop.propertyArray->prefixOnly)
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
        if (prop.propertyArray)
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
                if (auto subProp = findProperty(substring, prop.propertyArray))
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
    // Do not check for unknown properties if Ice prefix, ie Ice, Glacier2, etc
    for (const auto& props : PropertyNames::validProps)
    {
        string icePrefix = string(props.name) + ".";
        if (prefix.find(icePrefix) == 0)
        {
            return;
        }
    }

    vector<string> unknownProps;
    PropertyDict props = properties->getPropertiesForPrefix(string{prefix} + ".");
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
        for (const auto& prop : unknownProps)
        {
            os << "\n    " << prop;
        }

        throw PropertyException{__FILE__, __LINE__, os.str()};
    }
}
