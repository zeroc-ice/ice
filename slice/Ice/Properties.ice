// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h", "objc:header-dir:objc"]]

#include <Ice/PropertiesAdmin.ice>

["objc:prefix:ICE"]
module Ice
{

/**
 *
 * A property set used to configure Ice and Ice applications.
 * Properties are key/value pairs, with both keys and values
 * being strings. By convention, property keys should have the form
 * <em>application-name</em>\[.<em>category</em>\[.<em>sub-category</em>]].<em>name</em>.
 *
 **/
local interface Properties
{
    /**
     *
     * Get a property by key. If the property is not set, an empty
     * string is returned.
     *
     * @param key The property key.
     *
     * @return The property value.
     *
     * @see #setProperty
     *
     **/
    string getProperty(string key);

    /**
     *
     * Get a property by key. If the property is not set, the
     * given default value is returned.
     *
     * @param key The property key.
     *
     * @param value The default value to use if the property does not
     * exist.
     *
     * @return The property value or the default value.
     *
     * @see #setProperty
     *
     **/
    string getPropertyWithDefault(string key, string value);

    /**
     *
     * Get a property as an integer. If the property is not set, 0
     * is returned.
     *
     * @param key The property key.
     *
     * @return The property value interpreted as an integer.
     *
     * @see #setProperty
     *
     **/
    int getPropertyAsInt(string key);

    /**
     *
     * Get a property as an integer. If the property is not set, the
     * given default value is returned.
     *
     * @param key The property key.
     *
     * @param value The default value to use if the property does not
     * exist.
     *
     * @return The property value interpreted as an integer, or the
     * default value.
     *
     * @see #setProperty
     *
     **/
    int getPropertyAsIntWithDefault(string key, int value);


     /**
     *
     * Get a property as a list of strings. The strings must be
     * separated by whitespace or comma. If the property is not set,
     * an empty list is returned. The strings in the list can contain
     * whitespace and commas if they are enclosed in single or double 
     * quotes. If quotes are mismatched, an empty list is returned.
     * Within single quotes or double quotes, you can escape the
     * quote in question with \, e.g. O'Reilly can be written as
     * O'Reilly, "O'Reilly" or 'O\'Reilly'.
     *
     * @param key The property key.
     *
     * @return The property value interpreted as a list of strings.
     *
     * @see #setProperty
     *
     **/
    StringSeq getPropertyAsList(string key);

    /**
     *
     * Get a property as a list of strings.  The strings must be
     * separated by whitespace or comma. If the property is not set,
     * the default list is returned. The strings in the list can contain
     * whitespace and commas if they are enclosed in single or double 
     * quotes. If quotes are mismatched, the default list is returned.
     * Within single quotes or double quotes, you can escape the
     * quote in question with \, e.g. O'Reilly can be written as
     * O'Reilly, "O'Reilly" or 'O\'Reilly'.
     *
     * @param key The property key.
     *
     * @param value The default value to use if the property is not set.
     *
     * @return The property value interpreted as list of strings, or the
     * default value.
     *
     * @see #setProperty
     *
     **/
    StringSeq getPropertyAsListWithDefault(string key, StringSeq value);

    /**
     *
     * Get all properties whose keys begins with
     * <em>prefix</em>. If
     * <em>prefix</em> is an empty string,
     * then all properties are returned.
     *
     * @param prefix The prefix to search for (empty string if none).
     * @return The matching property set.
     *
     **/
    PropertyDict getPropertiesForPrefix(string prefix);

    /**
     *
     * Set a property. To unset a property, set it to
     * the empty string.
     *
     * @param key The property key.
     * @param value The property value.
     *
     * @see #getProperty
     *
     **/
    void setProperty(string key, string value);

    /**
     *
     * Get a sequence of command-line options that is equivalent to
     * this property set. Each element of the returned sequence is
     * a command-line option of the form
     * <tt>--<em>key</em>=<em>value</em></tt>.
     *
     * @return The command line options for this property set.
     *
     **/
    StringSeq getCommandLineOptions();

    /**
     *
     * Convert a sequence of command-line options into properties.
     * All options that begin with
     * <tt>--<em>prefix</em>.</tt> are
     * converted into properties. If the prefix is empty, all options
     * that begin with <tt>--</tt> are converted to properties.
     *
     * @param prefix The property prefix, or an empty string to
     * convert all options starting with <tt>--</tt>.
     *
     * @param options The command-line options.
     *
     * @return The command-line options that do not start with the specified
     * prefix, in their original order.
     *
     **/
    StringSeq parseCommandLineOptions(string prefix, StringSeq options);

    /**
     *
     * Convert a sequence of command-line options into properties.
     * All options that begin with one of the following prefixes
     * are converted into properties: <tt>--Ice</tt>, <tt>--IceBox</tt>, <tt>--IceGrid</tt>,
     * <tt>--IcePatch2</tt>, <tt>--IceSSL</tt>, <tt>--IceStorm</tt>, <tt>--Freeze</tt>, and <tt>--Glacier2</tt>.
     *
     * @param options The command-line options.
     *
     * @return The command-line options that do not start with one of
     * the listed prefixes, in their original order.
     *
     **/
    StringSeq parseIceCommandLineOptions(StringSeq options);

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

