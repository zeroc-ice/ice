// Copyright (c) ZeroC, Inc.

#ifndef ICE_PROPERTY_UTIL_H
#define ICE_PROPERTY_UTIL_H

#include "PropertyNames.h"

#include <optional>

namespace IceInternal
{
    /// Searches a property array for a property with the given key.
    /// @param key The key to search for.
    /// @param propertyArray The property array to search.
    /// @return The property if found, nullopt otherwise.
    std::optional<Property> findProperty(std::string_view key, const PropertyArray* propertyArray);
}

#endif
