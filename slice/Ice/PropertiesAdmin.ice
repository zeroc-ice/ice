// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["js:module:@zeroc/ice"]]
[["python:pkgdir:Ice"]]

#include "PropertyDict.ice"

["java:identifier:com.zeroc.Ice"]
module Ice
{
    /// Provides remote access to the properties of a communicator.
    interface PropertiesAdmin
    {
        /// Gets a property by key.
        /// @param key The property key.
        /// @return The property value. This value is empty if the property is not set.
        string getProperty(string key);

        /// Gets all properties whose keys begin with @p prefix. If @p prefix is the empty string then all properties
        /// are returned.
        /// @param prefix The prefix to search for. May be empty.
        /// @return The matching property set.
        ["java:type:java.util.TreeMap<String, String>"] PropertyDict getPropertiesForPrefix(string prefix);

        /// Updates the communicator's properties with the given property set. If an entry in @p newProperties matches
        /// the name of an existing property, that property's value is replaced with the new value. If the new value is
        /// the empty string, the property is removed. Existing properties that are not modified or removed by the
        /// entries in @p newProperties are not affected by this update.
        /// @param newProperties Properties to add, change, or remove.
        void setProperties(PropertyDict newProperties);
    }
}
