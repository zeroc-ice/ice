// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["js:module:@zeroc/ice"]]

[["python:pkgdir:Ice"]]

#include "PropertyDict.ice"

[["java:package:com.zeroc"]]

module Ice
{
    /// The PropertiesAdmin interface provides remote access to the properties of a communicator.
    interface PropertiesAdmin
    {
        /// Get a property by key. If the property is not set, an empty string is returned.
        /// @param key The property key.
        /// @return The property value.
        string getProperty(string key);

        /// Get all properties whose keys begin with <em>prefix</em>. If <em>prefix</em> is an empty string then all
        /// properties are returned.
        /// @param prefix The prefix to search for (empty string if none).
        /// @return The matching property set.
        ["java:type:java.util.TreeMap<String, String>"] PropertyDict getPropertiesForPrefix(string prefix);

        /// Update the communicator's properties with the given property set. If an entry in <em>newProperties</em>
        /// matches the name of an existing property, that property's value is replaced with the new value. If the new
        /// value is an empty string, the property is removed. Any existing properties that are not modified or removed
        /// by the entries in newProperties are retained with their original values.
        /// @param newProperties Properties to be added, changed, or removed.
        void setProperties(PropertyDict newProperties);
    }
}
