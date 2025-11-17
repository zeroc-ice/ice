// Copyright (c) ZeroC, Inc.

#ifndef ICE_PROPERTY_UTIL_H
#define ICE_PROPERTY_UTIL_H

#include "Ice/PropertiesF.h"
#include "Ice/PropertyDict.h"
#include "PropertyNames.h"

#include <optional>

namespace IceInternal
{
    /// Searches a property array for a property with the given key.
    /// @param key The key to search for.
    /// @param propertyArray The property array to search.
    /// @return The property if found, nullopt otherwise.
    std::optional<Property> findProperty(std::string_view key, const PropertyArray* propertyArray);

    /// Validates the properties for a given prefix.
    /// @param prefix The prefix to validate.
    /// @param properties The properties to consider.
    /// @param propertyArray The property array to search against.
    /// @throws PropertyException Thrown when an unknown property is found.
    void validatePropertiesWithPrefix(
        std::string_view prefix,
        const Ice::PropertiesPtr& properties,
        const PropertyArray* propertyArray);
}

#endif
