// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_PROPERTIES_ICE
#define ICE_PROPERTIES_ICE

#include <Ice/BuiltinSequences.ice>

module Ice
{

/**
 *
 * A simple collection of properties, represented as a dictionary of
 * key/value pairs. Both key and value are [string]s.
 *
 * @see Properties::getPropertiesForPrefix
 *
 **/
local dictionary<string, string> PropertyDict;

/**
 *
 * A property set to configure &Ice; and applications based on
 * &Ice;. Properties are key/value pairs, with both keys and values
 * being strings. By conventions, property keys should have the form
 * <replaceable>application-name</replaceable>\[.<replaceable>category</replaceable>\[.<replaceable>sub-category</replaceable>]].<replaceable>name</replaceable>.
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
     * Get a property by key. If the property does not exist, a
     * default value is returned.
     *
     * @param key The property key.
     *
     * @param value The supplied default value.
     *
     * @return The property value or the supplied default value.
     *
     * @see setProperty
     *
     **/
    string getPropertyWithDefault(string key, string value);

    /**
     *
     * Get a property as an integer. If the property does not exist, 0
     * is returned.
     *
     * @param key The property key.
     *
     * @return The property value interpreted as integer.
     *
     * @see setProperty
     *
     **/
    int getPropertyAsInt(string key);

    /**
     *
     * Get a property as an integer. If the property does not exist, a
     * default value is returned.
     *
     * @param key The property key.
     *
     * @param value The supplied default value.
     *
     * @return The property value interpreted as integer or the
     * supplied default value.
     *
     * @see setProperty
     *
     **/
    int getPropertyAsIntWithDefault(string key, int value);

    /**
     *
     * Get all properties whose keys begins with
     * <replaceable>prefix</replaceable>. If
     * <replaceable>prefix</replaceable> is an empty string,
     * then all properties are returned.
     *
     * @return The matching property set.
     *
     **/
    PropertyDict getPropertiesForPrefix(string prefix);

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
     * Get a sequence of command-line options that is equivalent to
     * this property set.
     *
     * @return The command line options for this property set.
     *
     **/
    StringSeq getCommandLineOptions();

    /**
     *
     * Convert a sequence of command-line options into properties.
     * All options which begin with
     * <screen>--<emphasis>prefix</emphasis>.</screen> will be
     * converted into properties. If the prefix is empty, all options
     * that begin with <screen>--</screen> will be converted to
     * properties.
     *
     * @param prefix The property prefix, or an empty string to
     * convert all options starting with <screen>--</screen>.
     *
     * @param options The command-line options.
     *
     * @return The command-line options which were not converted,
     * in the same order.
     *
     **/
    StringSeq parseCommandLineOptions(string prefix, StringSeq options);

    /**
     *
     * Load properties from a file.
     *
     * @param file The property file.
     *
     **/
    void load(string file);

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
