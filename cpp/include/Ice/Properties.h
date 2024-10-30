//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_PROPERTIES_H
#define ICE_PROPERTIES_H

#include "Config.h"
#include "Ice/BuiltinSequences.h"
#include "Ice/PropertyDict.h"
#include "PropertiesF.h"
#include "StringConverter.h"

#include <mutex>
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
         * Constructs an empty property set with a list of opt-in prefixes.
         * @param optInPrefixes The list of opt-in prefixes to allow in the property set.
         */
        explicit Properties(std::vector<std::string> optInPrefixes) : _optInPrefixes(std::move(optInPrefixes)) {}

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
        Properties(StringSeq& args, const PropertiesPtr& defaults);

        Properties& operator=(const Properties& rhs) = delete;

        /**
         * Get a property by key. If the property is not set, an empty string is returned.
         * @param key The property key.
         * @return The property value.
         * @see #setProperty
         */
        std::string getProperty(std::string_view key) noexcept;

        /**
         * Get an Ice property by key. If the property is not set, its default value is returned.
         * @param key The property key.
         * @return The property value or the default value.
         * @throws std::invalid_argument If the property is not a known Ice property.
         * @see #setProperty
         */
        std::string getIceProperty(std::string_view key);

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
         * Get an Ice property as an integer. If the property is not set, its default value is returned.
         * @param key The property key.
         * @return The property value interpreted as an integer, or the default value.
         * @throws std::invalid_argument If the property is not a known Ice property.
         * @see #setProperty
         */
        int getIcePropertyAsInt(std::string_view key);

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
         * enclosed in single or double quotes. If quotes are mismatched, an empty list is returned. Within single
         * quotes or double quotes, you can escape the quote in question with a backslash, e.g. O'Reilly can be written
         * as O'Reilly, "O'Reilly" or 'O\'Reilly'.
         * @param key The property key.
         * @return The property value interpreted as a list of strings.
         * @see #setProperty
         */
        StringSeq getPropertyAsList(std::string_view key) noexcept;

        /**
         * Get an Ice property as a list of strings. The strings must be separated by whitespace or comma. If the
         * property is not set, its default list is returned. The strings in the list can contain whitespace and commas
         * if they are enclosed in single or double quotes. If quotes are mismatched, the default list is returned.
         * Within single quotes or double quotes, you can escape the quote in question with a backslash, e.g. O'Reilly
         * can be written as O'Reilly, "O'Reilly" or 'O\'Reilly'.
         * @param key The property key.
         * @return The property value interpreted as list of strings, or the default value.
         * @throws std::invalid_argument If the property is not a known Ice property.
         * @see #setProperty
         */
        StringSeq getIcePropertyAsList(std::string_view key);

        /**
         * Get a property as a list of strings.  The strings must be separated by whitespace or comma. If the property
         * is not set, the default list is returned. The strings in the list can contain whitespace and commas if they
         * are enclosed in single or double quotes. If quotes are mismatched, the default list is returned. Within
         * single quotes or double quotes, you can escape the quote in question with a backslash, e.g. O'Reilly can be
         * written as O'Reilly, "O'Reilly" or 'O\'Reilly'.
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
         * <code>--<em>prefix</em>.</code> are converted into properties. If the prefix is empty, all options that begin
         * with <code>--</code> are converted to properties.
         * @param prefix The property prefix, or an empty string to convert all options starting with <code>--</code>.
         * @param options The command-line options.
         * @return The command-line options that do not start with the specified prefix, in their original order.
         */
        StringSeq parseCommandLineOptions(std::string_view prefix, const StringSeq& options);

        /**
         * Convert a sequence of command-line options into properties. All options that begin with one of the following
         * prefixes are converted into properties: <code>--Ice</code>, <code>--IceBox</code>, <code>--IceGrid</code>,
         * <code>--IceSSL</code>, <code>--IceStorm</code>, and <code>--Glacier2</code>.
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
        PropertiesPtr clone() { return std::make_shared<Properties>(*this); }

        /**
         * Get the properties that were never read.
         * @return A list of unused properties.
         */
        std::set<std::string> getUnusedProperties();

        /**
         * Parse a sequence of options into a map of key value pairs starting with a prefix. The options are expected to
         * be of the form <code><em>key</em>=<em>value</em></code>.
         * @param prefix The prefix to match.
         * @param options The options to parse.
         * @return A pair containing a map of matched key value pairs and a sequence of unmatched options.
         */
        static std::pair<std::map<std::string, std::string>, StringSeq>
        parseOptions(std::string_view prefix, const StringSeq& options);

    private:
        static std::optional<std::pair<std::string, std::string>>
        parseLine(std::string_view, const StringConverterPtr&);

        void loadArgs(StringSeq&);

        void loadConfig();

        struct PropertyValue
        {
            PropertyValue() : used(false) {}

            PropertyValue(std::string v, bool u) : value(std::move(v)), used(u) {}

            std::string value;
            bool used;
        };
        std::map<std::string, PropertyValue, std::less<>> _properties;
        // List of "opt-in" property prefixes to allow in the property set. Setting a property for a property prefix
        // that is opt-in (eg. IceGrid, IceStorm, Glacier2, etc.) but not in this list is considered an error.
        std::vector<std::string> _optInPrefixes;
        mutable std::mutex _mutex;
    };
}

#endif
