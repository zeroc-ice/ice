//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

export namespace Ice
{
    /**
     * A property set used to configure Ice and Ice applications.
     * Properties are key/value pairs, with both keys and values
     * being strings. By convention, property keys should have the form
     * <em>application-name</em>\[.<em>category</em>\[.<em>sub-category</em>]].<em>name</em>.
     */
    interface Properties
    {
        /**
         * Get a property by key. If the property is not set, an empty
         * string is returned.
         * @param key The property key.
         * @return The property value.
         * @see #setProperty
         */
        getProperty(key:string):string;
        /**
         * Get a property by key. If the property is not set, the
         * given default value is returned.
         * @param key The property key.
         * @param value The default value to use if the property does not
         * exist.
         * @return The property value or the default value.
         * @see #setProperty
         */
        getPropertyWithDefault(key:string, value:string):string;
        /**
         * Get a property as an integer. If the property is not set, 0
         * is returned.
         * @param key The property key.
         * @return The property value interpreted as an integer.
         * @see #setProperty
         */
        getPropertyAsInt(key:string):number;
        /**
         * Get a property as an integer. If the property is not set, the
         * given default value is returned.
         * @param key The property key.
         * @param value The default value to use if the property does not
         * exist.
         * @return The property value interpreted as an integer, or the
         * default value.
         * @see #setProperty
         */
        getPropertyAsIntWithDefault(key:string, value:number):number;
        /**
         * Get a property as a list of strings. The strings must be
         * separated by whitespace or comma. If the property is not set,
         * an empty list is returned. The strings in the list can contain
         * whitespace and commas if they are enclosed in single or double
         * quotes. If quotes are mismatched, an empty list is returned.
         * Within single quotes or double quotes, you can escape the
         * quote in question with \, e.g. O'Reilly can be written as
         * O'Reilly, "O'Reilly" or 'O\'Reilly'.
         * @param key The property key.
         * @return The property value interpreted as a list of strings.
         * @see #setProperty
         */
        getPropertyAsList(key:string):StringSeq;
        /**
         * Get a property as a list of strings.  The strings must be
         * separated by whitespace or comma. If the property is not set,
         * the default list is returned. The strings in the list can contain
         * whitespace and commas if they are enclosed in single or double
         * quotes. If quotes are mismatched, the default list is returned.
         * Within single quotes or double quotes, you can escape the
         * quote in question with \, e.g. O'Reilly can be written as
         * O'Reilly, "O'Reilly" or 'O\'Reilly'.
         * @param key The property key.
         * @param value The default value to use if the property is not set.
         * @return The property value interpreted as list of strings, or the
         * default value.
         * @see #setProperty
         */
        getPropertyAsListWithDefault(key:string, value:StringSeq):StringSeq;
        /**
         * Get all properties whose keys begins with
         * <em>prefix</em>. If
         * <em>prefix</em> is an empty string,
         * then all properties are returned.
         * @param prefix The prefix to search for (empty string if none).
         * @return The matching property set.
         */
        getPropertiesForPrefix(prefix:string):PropertyDict;
        /**
         * Set a property. To unset a property, set it to
         * the empty string.
         * @param key The property key.
         * @param value The property value.
         * @see #getProperty
         */
        setProperty(key:string, value:string):void;
        /**
         * Get a sequence of command-line options that is equivalent to
         * this property set. Each element of the returned sequence is
         * a command-line option of the form
         * <code>--<em>key</em>=<em>value</em></code>.
         * @return The command line options for this property set.
         */
        getCommandLineOptions():StringSeq;
        /**
         * Convert a sequence of command-line options into properties.
         * All options that begin with
         * <code>--<em>prefix</em>.</code> are
         * converted into properties. If the prefix is empty, all options
         * that begin with <code>--</code> are converted to properties.
         * @param prefix The property prefix, or an empty string to
         * convert all options starting with <code>--</code>.
         * @param options The command-line options.
         * @return The command-line options that do not start with the specified
         * prefix, in their original order.
         */
        parseCommandLineOptions(prefix:string, options:StringSeq):StringSeq;
        /**
         * Convert a sequence of command-line options into properties.
         * All options that begin with one of the following prefixes
         * are converted into properties: <code>--Ice</code>, <code>--IceBox</code>, <code>--IceGrid</code>,
         * <code>--IcePatch2</code>, <code>--IceSSL</code>, <code>--IceStorm</code>, <code>--Freeze</code>, and <code>--Glacier2</code>.
         * @param options The command-line options.
         * @return The command-line options that do not start with one of
         * the listed prefixes, in their original order.
         */
        parseIceCommandLineOptions(options:StringSeq):StringSeq;
        /**
         * Create a copy of this property set.
         * @return A copy of this property set.
         */
        clone():Ice.Properties;
    }
}
