// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PROPERTIES_ICE
#define ICE_PROPERTIES_ICE

module Ice
{

/**
 *
 * A property set to configure Ice and applications based on
 * Ice. Properties are key/value pairs, with both keys and values
 * being strings. By conventions, property keys should have the form
 * <replaceable>application-name</replaceable>[[.<replaceable>category</replaceable>].<replaceable>sub-category</replaceable>].<replaceable>name</replaceable>.
 *
 **/
local interface Properties
{
    /**
     *
     * Get a property by key. If the property does not exist, an empty
     * string is returned.
     *
     * @param key The property key.
     *
     * @return The property value.
     *
     * @see setProperty
     *
     **/
    string getProperty(string key);

    /**
     *
     * Set a property.
     *
     * @param key The property key.
     * @param value The property value.
     *
     * @see getProperty
     *
     **/
    void setProperty(string key, string value);

    /**
     *
     * Create a copy of this property set.
     *
     * @return A copy of this property set.
     *
     **/
    Properties clone();
};

};

#endif
