// Copyright (c) ZeroC, Inc.

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
    /// Represents a set of properties used to configure Ice and Ice-based applications. A property is a key/value pair,
    /// where both the key and the value are strings. By convention, property keys should have the form
    /// `application-name>[.category[.sub-category]].name`.
    /// @remark This class is thread-safe: multiple threads can safely read and write the properties without their own
    /// synchronization.
    /// @headerfile Ice/Ice.h
    class ICE_API Properties final
    {
    public:
        /// Default constructor.
        Properties() = default;

        /// Constructs an empty property set with a list of opt-in prefixes.
        /// @param optInPrefixes The list of opt-in prefixes allowed in the property set.
        /// @remark This constructor is used by services such as IceGrid and IceStorm to parse properties with the
        /// `IceGrid` and `IceStorm` prefixes.
        explicit Properties(std::vector<std::string> optInPrefixes) : _optInPrefixes(std::move(optInPrefixes)) {}

        /// Copy constructor.
        /// @param source The property set to copy.
        Properties(const Properties& source);

        /// Constructs a property from command-line arguments and a default property set.
        /// @param args The command-line arguments. Property arguments are removed from this sequence.
        /// @param defaults The default property set.
        Properties(StringSeq& args, const PropertiesPtr& defaults);

        Properties& operator=(const Properties& rhs) = delete;

        /// Gets a property by key.
        /// @param key The property key.
        /// @return The property value, or the empty string if the property is not set.
        /// @see #setProperty
        std::string getProperty(std::string_view key);

        /// Gets an Ice property by key.
        /// @param key The property key.
        /// @return The property value, or the default value for this property if the property is not set.
        /// @throws std::invalid_argument Thrown when the property is not a known Ice property.
        /// @see #setProperty
        std::string getIceProperty(std::string_view key);

        /// Gets a property by key.
        /// @param key The property key.
        /// @param value The default value to return if the property is not set.
        /// @return The property value or the default value if the property is not set.
        /// @see #setProperty
        std::string getPropertyWithDefault(std::string_view key, std::string_view value);

        /// Gets a property as an integer.
        /// @param key The property key.
        /// @return The property value interpreted as an integer, or 0 if the property is not set.
        /// @throws PropertyException Thrown when the property value is not a valid integer.
        /// @see #setProperty
        int getPropertyAsInt(std::string_view key);

        /// Gets an Ice property as an integer.
        /// @param key The property key.
        /// @return The property value interpreted as an integer, or the default value if the property is not set.
        /// @throws PropertyException Thrown when the property is not a known Ice property or the value is not a valid
        /// integer.
        /// @see #setProperty
        int getIcePropertyAsInt(std::string_view key);

        /// Gets a property as an integer.
        /// @param key The property key.
        /// @param value The default value to return if the property does not exist.
        /// @return The property value interpreted as an integer, or the default value of the property is not set.
        /// @throws PropertyException Thrown when the property value is not a valid integer.
        /// @see #setProperty
        int getPropertyAsIntWithDefault(std::string_view key, int value);

        /// Gets a property as a list of strings. The strings must be separated by whitespace or comma. The strings in
        /// the list can contain whitespace and commas if they are enclosed in single or double quotes. If quotes are
        /// mismatched, an empty list is returned. Within single quotes or double quotes, you can escape the quote in
        /// question with a backslash, e.g. O'Reilly can be written as O'Reilly, "O'Reilly" or 'O\'Reilly'.
        /// @param key The property key.
        /// @return The property value interpreted as a list of strings, or an empty list if the property is not set.
        /// @see #setProperty
        StringSeq getPropertyAsList(std::string_view key);

        /// Gets an Ice property as a list of strings. The strings must be separated by whitespace or comma. The strings
        /// in the list can contain whitespace and commas if they are enclosed in single or double quotes. If quotes are
        /// mismatched, the default list is returned.  Within single quotes or double quotes, you can escape the quote
        /// in question with a backslash, e.g. O'Reilly can be written as O'Reilly, "O'Reilly" or 'O\'Reilly'.
        /// @param key The property key.
        /// @return The property value interpreted as list of strings, or the default value if the property is not set.
        /// @throws PropertyException If the property is not a known Ice property.
        /// @see #setProperty
        StringSeq getIcePropertyAsList(std::string_view key);

        /// Gets a property as a list of strings. The strings must be separated by whitespace or comma. The strings in
        /// the list can contain whitespace and commas if they are enclosed in single or double quotes. If quotes are
        /// mismatched, the default list is returned. Within single quotes or double quotes, you can escape the quote
        /// in question with a backslash, e.g. O'Reilly can be written as O'Reilly, "O'Reilly" or 'O\'Reilly'.
        /// @param key The property key.
        /// @param value The default value to use if the property is not set.
        /// @return The property value interpreted as list of strings, or the default value if the property is not set.
        /// @see #setProperty
        StringSeq getPropertyAsListWithDefault(std::string_view key, const StringSeq& value);

        /// Gets all properties whose keys begins with @p prefix. If @p prefix is the empty string, then all properties
        /// are returned.
        /// @param prefix The prefix to search for (empty string if none).
        /// @return The matching property set.
        PropertyDict getPropertiesForPrefix(std::string_view prefix);

        /// Sets a property. To unset a property, set it to the empty string.
        /// @param key The property key.
        /// @param value The property value.
        /// @see #getProperty
        void setProperty(std::string_view key, std::string_view value);

        /// Gets a sequence of command-line options that is equivalent to this property set. Each element of the
        /// returned sequence is a command-line option of the form `--key=value`.
        /// @return The command line options for this property set.
        StringSeq getCommandLineOptions();

        /// Converts a sequence of command-line options into properties. All options that start with `--prefix.` are
        /// converted into properties. If the prefix is empty, all options that begin with `--` are converted to
        /// properties.
        /// @param prefix The property prefix, or the empty string to convert all options starting with `--`.
        /// @param options The command-line options.
        /// @return The command-line options that do not start with the specified prefix, in their original order.
        StringSeq parseCommandLineOptions(std::string_view prefix, const StringSeq& options);

        /// Converts a sequence of command-line options into properties. All options that start with one of the
        /// reserved Ice prefixes (`--Ice`, `--IceSSL`, etc.) are converted into properties.
        /// @param options The command-line options.
        /// @return The command-line options that do not start with one of the reserved prefixes, in their original
        /// order.
        StringSeq parseIceCommandLineOptions(const StringSeq& options);

        /// Loads properties from a file.
        /// @param file The property file.
        void load(std::string_view file);

        /// Creates a copy of this property set.
        /// @return A copy of this property set.
        PropertiesPtr clone() { return std::make_shared<Properties>(*this); }

        /// Gets the properties that were never read.
        /// @return A list of unused properties.
        std::set<std::string> getUnusedProperties();

        /// Parses a sequence of options into a map of key value pairs starting with a prefix. The options are expected
        /// to be of the form `--key=value`.
        /// @param prefix The prefix to match.
        /// @param options The options to parse.
        /// @return A pair containing a map of matched key value pairs and a sequence of unmatched options.
        static std::pair<PropertyDict, StringSeq> parseOptions(std::string_view prefix, const StringSeq& options);

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

    /// Creates an empty property set.
    /// @return A new empty property set.
    ICE_API PropertiesPtr createProperties();

    /// Creates a property set initialized from command-line arguments and a default property set.
    /// @param seq The command-line arguments. This function parses arguments starting with `--` and one of the
    /// reserved prefixes (Ice, IceSSL, etc.) as properties and removes these elements from the list. If there is an
    /// argument starting with `--Ice.Config`, this function loads the specified configuration file. When the same
    /// property is set in a configuration file and through a command-line argument, the command-line setting takes
    /// precedence.
    /// @param defaults Default values for the property set. Settings in configuration files and the arguments override
    /// these defaults.
    /// @return A new property set initialized with the properties that were removed from the argument vector.
    ICE_API PropertiesPtr createProperties(StringSeq& seq, const PropertiesPtr& defaults = nullptr);

    /// Creates a property set initialized from command-line arguments and a default property set.
    /// @param argc The number of arguments in @p argv. When this function parses properties from @p argv, it
    /// reshuffles the arguments so that the remaining arguments start at the beginning of @p argv, and updates @p argc.
    /// @param argv The command-line arguments. This function parses arguments starting with `--` and one of the
    /// reserved prefixes (Ice, IceSSL, etc.) as properties. If there is an argument starting with `--Ice.Config`, this
    /// function loads the specified configuration file. When the same property is set in a configuration file and
    /// through a command-line argument, the command-line setting takes precedence.
    /// @param defaults Default values for the property set. Settings in configuration files and the arguments override
    /// these defaults.
    /// @return A new property set initialized with the properties that were removed from the argument vector.
    ICE_API PropertiesPtr createProperties(int& argc, const char* argv[], const PropertiesPtr& defaults = nullptr);

    /// @copydoc createProperties(int&, const char*[], const PropertiesPtr&)
    inline PropertiesPtr createProperties(int& argc, char* argv[], const PropertiesPtr& defaults = nullptr)
    {
        return createProperties(argc, const_cast<const char**>(argv), defaults);
    }

#if defined(_WIN32) || defined(ICE_DOXYGEN)
    /// @copydoc createProperties(int&, const char*[], const PropertiesPtr&)
    /// @remark Windows only.
    ICE_API PropertiesPtr createProperties(int& argc, const wchar_t* argv[], const PropertiesPtr& defaults = nullptr);

    /// @copydoc createProperties(int&, const char*[], const PropertiesPtr&)
    /// @remark Windows only.
    inline PropertiesPtr createProperties(int& argc, wchar_t* argv[], const PropertiesPtr& defaults = nullptr)
    {
        return createProperties(argc, const_cast<const wchar_t**>(argv), defaults);
    }
#endif
}

#endif
