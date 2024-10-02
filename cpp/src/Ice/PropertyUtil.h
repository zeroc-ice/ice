// Copyright (c) ZeroC, Inc.

#ifndef ICE_PROPERTY_UTIL_H
#define ICE_PROPERTY_UTIL_H

#include "PropertyNames.h"

#include <optional>

namespace IceInternal
{
    /// Searches a property array for a property with the given key.
    /// @param propertyArray The property array to search.
    /// @param key The key to search for.
    /// @return The property if found, nullopt otherwise.
    std::optional<Property> findInPropertyArray(const PropertyArray* propertyArray, std::string_view key);
}

#endif
