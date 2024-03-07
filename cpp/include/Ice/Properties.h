//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_PROPERTIES_H
#define ICE_PROPERTIES_H

#include "Config.h"
#include "Ice/BuiltinSequences.h"
#include "Ice/PropertyDict.h"
#include "StringConverter.h"

#include <set>
#include <string>
#include <string_view>

namespace Ice
{

/**
 * A property set used to configure Ice and Ice applications. Properties are key/value pairs, with both keys and
 * values being strings. By convention, property keys should have the form
 * <em>application-name</em>[.<em>category</em>[.<em>sub-category</em>]].<em>name</em>.
 * This class is thread-safe: multiple threads can safely read and write the properties without their own
 * synchronization.
 * \headerfile Ice/Ice.h
 */
class ICE_API Properties final
{
public:

    /**
     * Default constructor.
    */
    Properties() = default;

    /**
     * Copy constructor.
     * @param source The property set to copy.
     */
    Properties(const Properties& source);

    /**
     * Constructs a property from command-line arguments and a default property set.
     * @param args The command-line arguments. Property arguments are removed from this sequence.
     * @param defaults The default property set.
     */
    Properties(StringSeq& args, const std::shared_ptr<Properties>& defaults);

    Properties& operator=(const Properties& rhs) = delete;

    /**
     * Get a property by key. If the property is not set, an empty string is returned.
     * @param key The property key.
     * @return The property value.
     * @see #setProperty
     */
    std::string getProperty(std::string_view key) noexcept;

    /**
     * Get a property by key. If the property is not set, the given default value is returned.
     * @param key The property key.
     * @param value The default value to use if the property does not exist.
     * @return The property value or the default value.
     * @see #setProperty
     */
    std::string getPropertyWithDefault(std::string_view key, std::string_view value) noexcept;

    /**
     * Get a property as an integer. If the property is not set, 0 is returned.
     * @param key The property key.
     * @return The property value interpreted as an integer.
     * @see #setProperty
     */
    int getPropertyAsInt(std::string_view key) noexcept;

    /**
     * Get a property as an integer. If the property is not set, the given default value is returned.
     * @param key The property key.
     * @param value The default value to use if the property does not exist.
     * @return The property value interpreted as an integer, or the default value.
     * @see #setProperty
     */
    int getPropertyAsIntWithDefault(std::string_view key, int value) noexcept;

    /**
     * Get a property as a list of strings. The strings must be separated by whitespace or comma. If the property is
     * not set, an empty list is returned. The strings in the list can contain whitespace and commas if they are
     * enclosed in single or double quotes. If quotes are mismatched, an empty list is returned. Within single quotes
     * or double quotes, you can escape the quote in question with a backslash, e.g. O'Reilly can be written as
     * O'Reilly, "O'Reilly" or 'O\'Reilly'.
     * @param key The property key.
     * @return The property value interpreted as a list of strings.
     * @see #setProperty
     */
    StringSeq getPropertyAsList(std::string_view key) noexcept;

    /**
     * Get a property as a list of strings.  The strings must be separated by whitespace or comma. If the property is
     * not set, the default list is returned. The strings in the list can contain whitespace and commas if they are
     * enclosed in single or double quotes. If quotes are mismatched, the default list is returned. Within single
     * quotes or double quotes, you can escape the quote in question with a backslash, e.g. O'Reilly can be written as
     * O'Reilly, "O'Reilly" or 'O\'Reilly'.
     * @param key The property key.
     * @param value The default value to use if the property is not set.
     * @return The property value interpreted as list of strings, or the default value.
     * @see #setProperty
     */
    StringSeq getPropertyAsListWithDefault(std::string_view key, const StringSeq& value) noexcept;

    /**
     * Get all properties whose keys begins with <em>prefix</em>. If <em>prefix</em> is an empty string, then all
     * properties are returned.
     * @param prefix The prefix to search for (empty string if none).
     * @return The matching property set.
     */
    PropertyDict getPropertiesForPrefix(std::string_view prefix) noexcept;

    /**
     * Set a property. To unset a property, set it to the empty string.
     * @param key The property key.
     * @param value The property value.
     * @see #getProperty
     */
    void setProperty(std::string_view key, std::string_view value);

    /**
     * Get a sequence of command-line options that is equivalent to this property set. Each element of the returned
     * sequence is a command-line option of the form <code>--<em>key</em>=<em>value</em></code>.
     * @return The command line options for this property set.
     */
    StringSeq getCommandLineOptions() noexcept;

    /**
     * Convert a sequence of command-line options into properties. All options that begin with
     * <code>--<em>prefix</em>.</code> are converted into properties. If the prefix is empty, all options that begin with
     * <code>--</code> are converted to properties.
     * @param prefix The property prefix, or an empty string to convert all options starting with <code>--</code>.
     * @param options The command-line options.
     * @return The command-line options that do not start with the specified prefix, in their original order.
     */
    StringSeq parseCommandLineOptions(std::string_view prefix, const StringSeq& options);

    /**
     * Convert a sequence of command-line options into properties. All options that begin with one of the following
     * prefixes are converted into properties: <code>--Ice</code>, <code>--IceBox</code>, <code>--IceGrid</code>,
     * <code>--IcePatch2</code>, <code>--IceSSL</code>, <code>--IceStorm</code>, <code>--Freeze</code>, and <code>--Glacier2</code>.
     * @param options The command-line options.
     * @return The command-line options that do not start with one of the listed prefixes, in their original order.
     */
    StringSeq parseIceCommandLineOptions(const StringSeq& options);

    /**
     * Load properties from a file.
     * @param file The property file.
     */
    void load(std::string_view file);

    /**
     * Create a copy of this property set.
     * @return A copy of this property set.
     */
    std::shared_ptr<Properties> clone() { return std::make_shared<Properties>(*this); }

    /**
     * Get the properties that were never read.
     * @return A list of unused properties.
     */
    std::set<std::string> getUnusedProperties();

private:

    void parseLine(std::string_view, const StringConverterPtr&);

    void loadConfig();

    struct PropertyValue
    {
        PropertyValue() :
            used(false)
        {
        }

        PropertyValue(std::string_view v, bool u) :
            value(std::string{v}),
            used(u)
        {
        }

        std::string value;
        bool used;
    };
    std::map<std::string, PropertyValue, std::less<>> _properties;
    std::mutex _mutex;
};

using PropertiesPtr = std::shared_ptr<Properties>;

}

#endif
