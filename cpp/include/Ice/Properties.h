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
    /// Converts an argument vector into a string sequence.
    /// @param argc The number of arguments in argv.
    /// @param argv The arguments.
    /// @return A string sequence containing the arguments.
    ICE_API StringSeq argsToStringSeq(int argc, const char* const argv[]);

#if defined(_WIN32) || defined(ICE_DOXYGEN)
    /// @copydoc argsToStringSeq(int, const char* const[])
    /// @remark Windows only.
    ICE_API StringSeq argsToStringSeq(int argc, const wchar_t* const argv[]);
#endif

    /// Updates @p argv to match the contents of @p seq. This function assumes that @p seq contains only elements of
    /// @p argv. The function shifts the argument vector elements so that the vector matches the contents of the
    /// sequence.
    /// @param seq The string sequence returned from a call to #argsToStringSeq.
    /// @param[in,out] argc The number of arguments, updated to reflect the size of the sequence.
    /// @param argv The arguments, shifted to match @p seq.
    ICE_API void stringSeqToArgs(const StringSeq& seq, int& argc, const char* argv[]);

    /// @copydoc stringSeqToArgs(const StringSeq&, int&, const char*[])
    inline void stringSeqToArgs(const StringSeq& seq, int& argc, char* argv[])
    {
        return stringSeqToArgs(seq, argc, const_cast<const char**>(argv));
    }

#if defined(_WIN32) || defined(ICE_DOXYGEN)
    /// @copydoc stringSeqToArgs(const StringSeq&, int&, const char*[])
    /// @remark Windows only.
    ICE_API void stringSeqToArgs(const StringSeq& seq, int& argc, const wchar_t* argv[]);

    /// @copydoc stringSeqToArgs(const StringSeq&, int&, const char*[])
    /// @remark Windows only.
    inline void stringSeqToArgs(const StringSeq& seq, int& argc, wchar_t* argv[])
    {
        return stringSeqToArgs(seq, argc, const_cast<const wchar_t**>(argv));
    }
#endif

    /// Represents a set of properties used to configure Ice and Ice-based applications. A property is a key/value pair,
    /// where both the key and the value are strings. By convention, property keys should have the form
    /// `application-name[.category[.sub-category]].name`.
    /// @remark This class is thread-safe: multiple threads can safely read and write the properties.
    /// @headerfile Ice/Ice.h
    class ICE_API Properties final
    {
    public:
        /// Constructs an empty property set.
        Properties() = default;

        /// Constructs a property set, loads the configuration files specified by the `Ice.Config` property or the
        /// `ICE_CONFIG` environment variable, and then parses Ice properties from @p args.
        /// @param args The command-line arguments. This constructor parses arguments starting with `--` and one
        /// of the reserved prefixes (Ice, IceSSL, etc.) as properties and removes these elements from the vector. If
        /// there is an argument starting with `--Ice.Config`, this constructor loads the specified configuration file.
        /// When the same property is set in a configuration file and through a command-line argument, the command-line
        /// setting takes precedence.
        /// @param defaults Default values for the new Properties object. Settings in configuration files and the
        /// arguments override these defaults.
        /// @remarks This constructor loads properties from files specified by the `ICE_CONFIG` environment variable
        /// when there is no `--Ice.Config` command-line argument.
        explicit Properties(StringSeq& args, const PropertiesPtr& defaults = nullptr);

        /// Constructs a property set, loads the configuration files specified by the `Ice.Config` property or the
        /// `ICE_CONFIG` environment variable, and then parses Ice properties from @p args.
        /// @tparam ArgvT The type of the argument vector, such as char**, const char**, or wchar_t** (on Windows).
        /// @param[in, out] argc The number of command-line arguments in @p argv. When this constructor
        /// parses properties from @p argv, it reshuffles the arguments so that the remaining arguments start at the
        /// beginning of @p argv, and updates @p argc accordingly.
        /// @param argv The command-line arguments. This constructor parses arguments starting with `--` and one of the
        /// reserved prefixes (Ice, IceSSL, etc.) as properties. If there is an argument starting with `--Ice.Config`,
        /// this constructor loads the specified configuration file. When the same property is set in a configuration
        /// file and through a command-line argument, the command-line setting takes precedence.
        /// @param defaults Default values for the new Properties object. Settings in configuration files and the
        /// arguments override these defaults.
        /// @remarks This constructor loads properties from files specified by the `ICE_CONFIG` environment variable
        /// when there is no `--Ice.Config` command-line argument.
        template<typename ArgvT>
        Properties(int& argc, ArgvT argv, const PropertiesPtr& defaults = nullptr) : Properties{defaults}
        {
            StringSeq args = argsToStringSeq(argc, argv);
            loadArgs(args);
            stringSeqToArgs(args, argc, argv);
        }

        /// @private
        /// Constructs an empty property set with additional reserved prefixes such as "IceBox", "IceStorm", etc.
        /// @param firstOptInPrefix The first opt-in prefix.
        /// @param remainingOptInPrefixes The remaining opt-in prefixes.
        template<typename... T>
        Properties(std::string firstOptInPrefix, T... remainingOptInPrefixes)
            : _optInPrefixes{std::move(firstOptInPrefix), std::move(remainingOptInPrefixes)...}
        {
        }

        /// @private
        /// Constructs a property set, loads the configuration files specified by the `Ice.Config` property or the
        /// `ICE_CONFIG` environment variable, and then parses Ice properties from @p args.
        /// @tparam ArgvT The type of the argument vector, such as char**, const char**, or wchar_t** (on Windows).
        /// @param[in, out] argc The number of command-line arguments in @p argv.
        /// @param argv The command-line arguments. This constructor parses arguments starting with `--` and one of the
        /// reserved prefixes (Ice, IceSSL, etc.) as properties and moves these arguments to the end of the array. If
        /// there is an argument starting with `--Ice.Config`, this constructor loads the specified configuration file.
        /// When the same property is set in a configuration file and through a command-line argument, the command-line
        /// setting takes precedence.
        /// @param firstOptInPrefix The first opt-in prefix.
        /// @param remainingOptInPrefixes The remaining opt-in prefixes.
        /// @remarks This constructor loads properties from files specified by the `ICE_CONFIG` environment variable
        /// when there is no `--Ice.Config` command-line argument.
        template<typename ArgvT, typename... T>
        Properties(int& argc, ArgvT argv, std::string firstOptInPrefix, T... remainingOptInPrefixes)
            : Properties{std::move(firstOptInPrefix), std::move(remainingOptInPrefixes)...}
        {
            StringSeq args = argsToStringSeq(argc, argv);
            loadArgs(args);
            stringSeqToArgs(args, argc, argv);
        }

        /// Copy constructor.
        /// @param source The property set to copy.
        Properties(const Properties& source);

        Properties(Properties&&) = delete;
        Properties& operator=(const Properties& rhs) = delete;
        Properties& operator=(Properties&& rhs) = delete;

        /// Gets a property by key.
        /// @param key The property key.
        /// @return The property value, or the empty string if the property is not set.
        /// @see #setProperty
        std::string getProperty(std::string_view key);

        /// Gets an Ice property by key.
        /// @param key The property key.
        /// @return The property value, or the default value for this property if the property is not set.
        /// @throws PropertyException Thrown when the property is not a known Ice property.
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
        /// @return The property value interpreted as an integer, or the default value if the property is not set.
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
        /// mismatched, the default list is returned. Within single quotes or double quotes, you can escape the quote
        /// in question with a backslash, e.g. O'Reilly can be written as O'Reilly, "O'Reilly" or 'O\'Reilly'.
        /// @param key The property key.
        /// @return The property value interpreted as a list of strings, or the default value if the property is not
        ///     set.
        /// @throws PropertyException Thrown when the property is not a known Ice property.
        /// @see #setProperty
        StringSeq getIcePropertyAsList(std::string_view key);

        /// Gets a property as a list of strings. The strings must be separated by whitespace or comma. The strings in
        /// the list can contain whitespace and commas if they are enclosed in single or double quotes. If quotes are
        /// mismatched, the default list is returned. Within single quotes or double quotes, you can escape the quote
        /// in question with a backslash, e.g. O'Reilly can be written as O'Reilly, "O'Reilly" or 'O\'Reilly'.
        /// @param key The property key.
        /// @param value The default value to use if the property is not set.
        /// @return The property value interpreted as a list of strings, or the default value if the property is not
        ///     set.
        /// @see #setProperty
        StringSeq getPropertyAsListWithDefault(std::string_view key, const StringSeq& value);

        /// Gets all properties whose keys begin with @p prefix. If @p prefix is the empty string, then all properties
        /// are returned.
        /// @param prefix The prefix to search for.
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

        Properties(const PropertiesPtr& defaults);

        void loadArgs(StringSeq& args);
        void loadConfig();

        struct PropertyValue
        {
            PropertyValue() : used(false) {}

            PropertyValue(std::string v, bool u) : value(std::move(v)), used(u) {}

            std::string value;
            bool used;
        };

        std::map<std::string, PropertyValue, std::less<>> _propertySet;
        // List of "opt-in" property prefixes to allow in the property set. Setting a property for a property prefix
        // that is opt-in (eg. IceGrid, IceStorm, Glacier2, etc.) but not in this list is considered an error.
        std::vector<std::string> _optInPrefixes;
        mutable std::mutex _mutex;
    };

    /// Creates a new shared Properties object.
    /// @param args The arguments to forward to `make_shared<Properties>`.
    /// @return A new property set.
    /// @remarks This function is provided for backwards compatibility. New code should call
    /// `std::make_shared<Properties>` directly.
    template<class... T> inline PropertiesPtr createProperties(T&&... args)
    {
        return std::make_shared<Properties>(std::forward<T>(args)...);
    }
}

#endif
