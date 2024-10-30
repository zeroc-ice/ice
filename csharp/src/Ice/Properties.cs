// Copyright (c) ZeroC, Inc.

#nullable enable

using Ice.Internal;
using System.Globalization;
using System.Text;
using System.Text.RegularExpressions;

namespace Ice;

/// <summary>
/// A property set used to configure Ice and Ice applications. Properties are key/value pairs, with both keys and
/// values being strings.
/// </summary>
public sealed class Properties
{
    private class PropertyValue
    {
        public string value { get; set; }

        public bool used { get; set; }

        public PropertyValue(string value, bool used)
        {
            this.value = value;
            this.used = used;
        }

        public PropertyValue Clone() => new(value, used);
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="Properties" /> class. The property set is initially empty.
    /// </summary>
    public Properties()
    {
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="Properties" /> class. The property set is initialized from the
    /// provided argument vector.
    /// </summary>
    /// <param name="args">A command-line argument vector, possibly containing options to set properties. If the
    /// command-line options include a --Ice.Config option, the corresponding configuration files are parsed. If the
    /// same property is set in a configuration file and in the argument vector, the argument vector takes precedence.
    /// This method modifies the argument vector by removing any Ice-related options.</param>
    /// <param name="defaults">Default values for the property set. Settings in configuration files and args override
    /// these defaults. May be null.</param>
    public Properties(ref string[] args, Properties? defaults = null)
    {
        if (defaults is not null)
        {
            foreach (KeyValuePair<string, PropertyValue> entry in defaults._properties)
            {
                _properties[entry.Key] = entry.Value.Clone();
            }
        }

        if (_properties.TryGetValue("Ice.ProgramName", out PropertyValue? pv))
        {
            pv.used = true;
        }
        else
        {
            _properties["Ice.ProgramName"] = new PropertyValue(AppDomain.CurrentDomain.FriendlyName, true);
        }

        bool loadConfigFiles = false;

        for (int i = 0; i < args.Length; i++)
        {
            if (args[i].StartsWith("--Ice.Config", StringComparison.Ordinal))
            {
                string line = args[i];
                if (!line.Contains('=', StringComparison.Ordinal))
                {
                    line += "=1";
                }
                parseLine(line.Substring(2));
                loadConfigFiles = true;

                string[] arr = new string[args.Length - 1];
                Array.Copy(args, 0, arr, 0, i);
                if (i < args.Length - 1)
                {
                    Array.Copy(args, i + 1, arr, i, args.Length - i - 1);
                }
                args = arr;
            }
        }

        if (!loadConfigFiles)
        {
            //
            // If Ice.Config is not set, load from ICE_CONFIG (if set)
            //
            loadConfigFiles = !_properties.ContainsKey("Ice.Config");
        }

        if (loadConfigFiles)
        {
            loadConfig();
        }

        args = parseIceCommandLineOptions(args);
    }

    /// <summary>
    /// Get a property by key.
    /// If the property is not set, an empty string is returned.
    /// </summary>
    /// <param name="key">The property key.</param>
    /// <returns>The property value.</returns>
    public string getProperty(string key)
    {
        lock (_mutex)
        {
            string result = "";
            if (_properties.TryGetValue(key, out PropertyValue? pv))
            {
                pv.used = true;
                result = pv.value;
            }
            return result;
        }
    }

    /// <summary>
    /// Get an Ice property by key.
    /// If the property is not set, its default value is returned.
    /// </summary>
    /// <param name="key">The property key.</param>
    /// <returns>The property value or the default value.</returns>
    public string getIceProperty(string key) => getPropertyWithDefault(key, getDefaultProperty(key));

    /// <summary>
    /// Get a property by key.
    /// If the property is not set, the given default value is returned.
    /// </summary>
    /// <param name="key">The property key.</param>
    /// <param name="value">The default value to use if the property does not exist.</param>
    /// <returns>The property value or the default value.</returns>
    public string getPropertyWithDefault(string key, string value)
    {
        lock (_mutex)
        {
            string result = value;
            if (_properties.TryGetValue(key, out PropertyValue? pv))
            {
                pv.used = true;
                result = pv.value;
            }
            return result;
        }
    }

    /// <summary>
    /// Get a property as an integer.
    /// If the property is not set, 0 is returned.
    /// </summary>
    /// <param name="key">The property key.</param>
    /// <returns>The property value interpreted as an integer.</returns>
    public int getPropertyAsInt(string key) => getPropertyAsIntWithDefault(key, 0);

    /// <summary>
    /// Get an Ice property as an integer.
    /// If the property is not set, its default value is returned.
    /// </summary>
    /// <param name="key">The property key.</param>
    /// <returns>The property value interpreted as an integer, or the default value.</returns>
    public int getIcePropertyAsInt(string key)
    {
        string defaultValueString = getDefaultProperty(key);
        int defaultValue = 0;
        if (defaultValueString.Length > 0)
        {
            defaultValue = int.Parse(defaultValueString, CultureInfo.InvariantCulture);
        }

        return getPropertyAsIntWithDefault(key, defaultValue);
    }

    /// <summary>
    /// Get a property as an integer.
    /// If the property is not set, the given default value is returned.
    /// </summary>
    /// <param name="key">The property key.</param>
    /// <param name="value">The default value to use if the property does not exist.</param>
    /// <returns>The property value interpreted as an integer, or the default value.</returns>
    public int getPropertyAsIntWithDefault(string key, int value)
    {
        lock (_mutex)
        {
            if (!_properties.TryGetValue(key, out PropertyValue? pv))
            {
                return value;
            }
            pv.used = true;
            try
            {
                return int.Parse(pv.value, CultureInfo.InvariantCulture);
            }
            catch (FormatException)
            {
                Util.getProcessLogger().warning($"numeric property {key} set to non-numeric value, defaulting to {value}");
                return value;
            }
        }
    }

    /// <summary>
    /// Get a property as a list of strings.
    /// The strings must be separated by whitespace or comma. If the property is
    /// not set, an empty list is returned. The strings in the list can contain whitespace and commas if they are
    /// enclosed in single or double quotes. If quotes are mismatched, an empty list is returned. Within single quotes
    /// or double quotes, you can escape the quote in question with a backslash, e.g. O'Reilly can be written as
    /// O'Reilly, "O'Reilly" or 'O\'Reilly'.
    /// </summary>
    /// <param name="key">The property key.</param>
    /// <returns>The property value interpreted as a list of strings.</returns>
    public string[] getPropertyAsList(string key) => getPropertyAsListWithDefault(key, []);

    /// <summary>
    /// Get an Ice property as a list of strings.
    /// The strings must be separated by whitespace or comma. If the property is
    /// not set, its default list is returned. The strings in the list can contain whitespace and commas if they are
    /// enclosed in single or double quotes. If quotes are mismatched, the default list is returned. Within single
    /// quotes or double quotes, you can escape the quote in question with a backslash, e.g. O'Reilly can be written as
    /// O'Reilly, "O'Reilly" or 'O\'Reilly'.
    /// </summary>
    /// <param name="key">The property key.</param>
    /// <returns>The property value interpreted as list of strings, or the default value.</returns>
    public string[] getIcePropertyAsList(string key)
    {
        string[] defaultList = UtilInternal.StringUtil.splitString(getDefaultProperty(key), ", \t\r\n");
        return getPropertyAsListWithDefault(key, defaultList);
    }

    /// <summary>
    /// Get a property as a list of strings.
    /// The strings must be separated by whitespace or comma. If the property is
    /// not set, the default list is returned. The strings in the list can contain whitespace and commas if they are
    /// enclosed in single or double quotes. If quotes are mismatched, the default list is returned. Within single
    /// quotes or double quotes, you can escape the quote in question with a backslash, e.g. O'Reilly can be written as
    /// O'Reilly, "O'Reilly" or 'O\'Reilly'.
    /// </summary>
    /// <param name="key">The property key.</param>
    /// <param name="value">The default value to use if the property is not set.</param>
    /// <returns>The property value interpreted as list of strings, or the default value.</returns>
    public string[] getPropertyAsListWithDefault(string key, string[] value)
    {
        value ??= [];

        lock (_mutex)
        {
            if (!_properties.TryGetValue(key, out PropertyValue? pv))
            {
                return value;
            }

            pv.used = true;

            string[] result = Ice.UtilInternal.StringUtil.splitString(pv.value, ", \t\r\n");
            if (result == null)
            {
                Util.getProcessLogger().warning(
                    $"mismatched quotes in property {key}'s value, returning default value");
                return value;
            }
            else
            {
                return result;
            }
        }
    }

    /// <summary>
    /// Get all properties whose keys begins with prefix.
    /// If prefix is an empty string, then all
    /// properties are returned.
    /// </summary>
    /// <param name="prefix">The prefix to search for (empty string if none).</param>
    /// <returns>The matching property set.</returns>
    public Dictionary<string, string> getPropertiesForPrefix(string prefix)
    {
        lock (_mutex)
        {
            var result = new Dictionary<string, string>();

            foreach (string s in _properties.Keys)
            {
                if (prefix.Length == 0 || s.StartsWith(prefix, StringComparison.Ordinal))
                {
                    PropertyValue pv = _properties[s];
                    pv.used = true;
                    result[s] = pv.value;
                }
            }
            return result;
        }
    }

    /// <summary>
    /// Set a property.
    /// To unset a property, set it to the empty string.
    /// </summary>
    /// <param name="key">The property key.</param>
    /// <param name="value">The property value.</param>
    public void setProperty(string key, string value)
    {
        //
        // Trim whitespace
        //
        if (key != null)
        {
            key = key.Trim();
        }
        if (key == null || key.Length == 0)
        {
            throw new InitializationException("Attempt to set property with empty key");
        }

        // Check if the property is in an Ice property prefix. If so, check that it's a valid property.
        if (findIcePropertyArray(key) is PropertyArray propertyArray)
        {
            Property prop = findProperty(key[(propertyArray.name.Length + 1)..], propertyArray) ?? throw new PropertyException($"unknown Ice property: {key}");

            // If the property is deprecated, log a warning.
            if (prop.deprecated)
            {
                Util.getProcessLogger().warning($"setting deprecated property: {key}");
            }
        }

        lock (_mutex)
        {
            // Set or clear the property.
            if (value is not null && value.Length > 0)
            {
                if (_properties.TryGetValue(key, out PropertyValue? pv))
                {
                    pv.value = value;
                }
                else
                {
                    pv = new PropertyValue(value, false);
                }
                _properties[key] = pv;
            }
            else
            {
                _properties.Remove(key);
            }
        }
    }

    /// <summary>
    /// Get a sequence of command-line options that is equivalent to this property set.
    /// Each element of the returned
    /// sequence is a command-line option of the form --key=value.
    /// </summary>
    /// <returns>The command line options for this property set.</returns>
    public string[] getCommandLineOptions()
    {
        lock (_mutex)
        {
            string[] result = new string[_properties.Count];
            int i = 0;
            foreach (KeyValuePair<string, PropertyValue> entry in _properties)
            {
                result[i++] = "--" + entry.Key + "=" + entry.Value.value;
            }
            return result;
        }
    }

    /// <summary>
    /// Convert a sequence of command-line options into properties.
    /// All options that begin with
    /// --prefix. are converted into properties. If the prefix is empty, all options that begin with
    /// -- are converted to properties.
    /// </summary>
    /// <param name="prefix">The property prefix, or an empty string to convert all options starting with --.</param>
    /// <param name="options">The command-line options.</param>
    /// <returns>The command-line options that do not start with the specified prefix, in their original order.</returns>
    public string[] parseCommandLineOptions(string prefix, string[] options)
    {
        if (prefix.Length > 0 && prefix[^1] != '.')
        {
            prefix += '.';
        }
        prefix = "--" + prefix;

        List<string> result = [];
        for (int i = 0; i < options.Length; i++)
        {
            string opt = options[i];
            if (opt.StartsWith(prefix, StringComparison.Ordinal))
            {
                if (!opt.Contains('=', StringComparison.Ordinal))
                {
                    opt += "=1";
                }

                parseLine(opt.Substring(2));
            }
            else
            {
                result.Add(opt);
            }
        }
        string[] arr = new string[result.Count];
        if (arr.Length != 0)
        {
            result.CopyTo(arr);
        }
        return arr;
    }

    /// <summary>
    /// Convert a sequence of command-line options into properties.
    /// All options that begin with one of the following
    /// prefixes are converted into properties: --Ice, --IceBox, --IceGrid,
    /// --IceSSL, --IceStorm, and --Glacier2.
    /// </summary>
    /// <param name="options">The command-line options.</param>
    /// <returns>The command-line options that do not start with one of the listed prefixes, in their original order.
    /// </returns>
    public string[] parseIceCommandLineOptions(string[] options)
    {
        string[] args = options;
        foreach (string? name in PropertyNames.validProps.Select(p => p.name))
        {
            args = parseCommandLineOptions(name, args);
        }
        return args;
    }

    /// <summary>
    /// Load properties from a file.
    /// </summary>
    /// <param name="file">The property file.</param>
    public void load(string file)
    {
        try
        {
            using var sr = new StreamReader(file);
            parse(sr);
        }
        catch (IOException ex)
        {
            throw new FileException($"Cannot read '{file}'", ex);
        }
    }

    /// <summary>
    /// Create a copy of this property set.
    /// </summary>
    /// <returns>A copy of this property set.</returns>
    public Properties Clone()
    {
        var clonedProperties = new Properties();
        lock (_mutex)
        {
            foreach ((string key, PropertyValue value) in _properties)
            {
                clonedProperties._properties[key] = value.Clone();
            }
        }
        return clonedProperties;
    }

    /// <summary>
    /// Alias for <see cref="Clone" />. Provided for source compatibility with Ice 3.7 and earlier versions.
    /// </summary>
    /// <returns>A copy of this property set.</returns>
    [Obsolete("Use Clone instead.")]
    public Properties ice_clone_() => Clone();

    /// <summary>
    /// Get the unused properties in the property set.
    /// </summary>
    /// <returns>A list containing the names of the unused properties in this property set.</returns>
    public List<string> getUnusedProperties()
    {
        lock (_mutex)
        {
            var unused = new List<string>();
            foreach ((string key, PropertyValue value) in _properties)
            {
                if (!value.used)
                {
                    unused.Add(key);
                }
            }
            return unused;
        }
    }

    /// <summary>
    /// Validate the properties for the given prefix.
    /// </summary>
    /// <param name="prefix">The property prefix to validate.</param>
    /// <param name="properties">The properties to consider. </param>
    /// <param name="propertyArray">The property array to search against.</param>
    /// <exception cref="PropertyException"> Thrown if unknown properties are found.</exception>
    internal static void validatePropertiesWithPrefix(string prefix, Properties properties, PropertyArray propertyArray)
    {
        // Do not check for unknown properties if Ice prefix, ie Ice, Glacier2, etc
        foreach (string? name in PropertyNames.validProps.Select(p => p.name))
        {
            if (prefix.StartsWith($"{name}.", StringComparison.Ordinal))
            {
                return;
            }
        }

        var unknownProps = new List<string>();
        Dictionary<string, string> props = properties.getPropertiesForPrefix($"{prefix}.");

        foreach (string p in props.Keys)
        {
            // Plus one to include the dot.
            if (findProperty(p[(prefix.Length + 1)..], propertyArray) is null)
            {
                unknownProps.Add(p);
            }
        }

        if (unknownProps.Count > 0)
        {
            var message = new StringBuilder($"Found unknown properties for {propertyArray.name} : '{prefix}'");
            foreach (string s in unknownProps)
            {
                message.Append("\n    ");
                message.Append(s);
            }

            throw new PropertyException(message.ToString());
        }
    }

    /// <summary>
    /// Find the Ice property array for a given property name.
    /// </summary>
    /// <param name="key">The property key.</param>
    /// <returns>The Ice property array if found, else null.</returns>
    private static PropertyArray? findIcePropertyArray(string key)
    {
        int dotPos = key.IndexOf('.', StringComparison.Ordinal);

        // If the key doesn't contain a dot, it's not a valid Ice property
        if (dotPos == -1)
        {
            return null;
        }

        string prefix = key[..dotPos];

        // Search for the property list that matches the prefix
        return PropertyNames.validProps.FirstOrDefault(properties => prefix == properties.name);
    }

    /// <summary>
    /// Find a property in the property array.
    /// </summary>
    /// <param name="key">The property key.</param>
    /// <param name="propertyArray">The property array to check.</param>
    /// <returns>The <see cref="Property" /> if found, null otherwise.</returns>
    private static Property? findProperty(string key, PropertyArray propertyArray)
    {
        foreach (Property prop in propertyArray.properties)
        {
            // If the key is an exact match, return the property unless it has a property class which is prefix only.
            // If the key is a regex match, return the property. A property cannot have a property class and use regex.
            if (key == prop.pattern)
            {
                if (prop.propertyArray is not null && prop.propertyArray.prefixOnly)
                {
                    return null;
                }
                return prop;
            }
            else if (prop.usesRegex && Regex.IsMatch(key, prop.pattern))
            {
                return prop;
            }

            // If the property has a property class, check if the key is a prefix of the property.
            if (prop.propertyArray is not null)
            {
                // Check if the key is a prefix of the property.
                // The key must be:
                // - shorter than the property pattern
                // - the property pattern must start with the key
                // - the pattern character after the key must be a dot
                if (key.Length > prop.pattern.Length && key.StartsWith($"{prop.pattern}.", StringComparison.Ordinal))
                {
                    // Plus one to skip the dot.
                    string substring = key[(prop.pattern.Length + 1)..];

                    // Check if the suffix is a valid property. If so, return it. If it's not, continue searching
                    // the current property array.
                    if (findProperty(substring, prop.propertyArray) is Property subProp)
                    {
                        return subProp;
                    }
                }
            }
        }
        return null;
    }

    /// <summary>
    /// Gets the default value for a given Ice property.
    /// </summary>
    /// <param name="key">The Ice property key.</param>
    /// <returns>The default property value, or an empty string the default is unspecified.</returns>
    /// <exception cref="ArgumentException">Thrown if the given key doesn't represent the name of an Ice property.
    /// </exception>
    private static string getDefaultProperty(string key)
    {
        // Find the property, don't log any warnings.
        PropertyArray propertyArray = findIcePropertyArray(key) ?? throw new PropertyException($"unknown Ice property: {key}");

        // Find the property in the property array.
        Property prop = findProperty(key[(propertyArray.name.Length + 1)..], propertyArray) ?? throw new PropertyException($"unknown Ice property: {key}");

        return prop.defaultValue;
    }

    private void parse(StreamReader input)
    {
        try
        {
            string? line;
            while ((line = input.ReadLine()) is not null)
            {
                parseLine(line);
            }
        }
        catch (IOException ex)
        {
            throw new SyscallException(ex);
        }
    }

    private const int ParseStateKey = 0;
    private const int ParseStateValue = 1;

    private void parseLine(string line)
    {
        string key = "";
        string val = "";

        int state = ParseStateKey;

        string whitespace = "";
        string escapedSpace = "";
        bool finished = false;
        for (int i = 0; i < line.Length; ++i)
        {
            char c = line[i];
            switch (state)
            {
                case ParseStateKey:
                {
                    switch (c)
                    {
                        case '\\':
                            if (i < line.Length - 1)
                            {
                                c = line[++i];
                                switch (c)
                                {
                                    case '\\':
                                    case '#':
                                    case '=':
                                        key += whitespace;
                                        whitespace = "";
                                        key += c;
                                        break;

                                    case ' ':
                                        if (key.Length != 0)
                                        {
                                            whitespace += c;
                                        }
                                        break;

                                    default:
                                        key += whitespace;
                                        whitespace = "";
                                        key += '\\';
                                        key += c;
                                        break;
                                }
                            }
                            else
                            {
                                key += whitespace;
                                key += c;
                            }
                            break;

                        case ' ':
                        case '\t':
                        case '\r':
                        case '\n':
                            if (key.Length != 0)
                            {
                                whitespace += c;
                            }
                            break;

                        case '=':
                            whitespace = "";
                            state = ParseStateValue;
                            break;

                        case '#':
                            finished = true;
                            break;

                        default:
                            key += whitespace;
                            whitespace = "";
                            key += c;
                            break;
                    }
                    break;
                }

                case ParseStateValue:
                {
                    switch (c)
                    {
                        case '\\':
                            if (i < line.Length - 1)
                            {
                                c = line[++i];
                                switch (c)
                                {
                                    case '\\':
                                    case '#':
                                    case '=':
                                        val += val.Length == 0 ? escapedSpace : whitespace;
                                        whitespace = "";
                                        escapedSpace = "";
                                        val += c;
                                        break;

                                    case ' ':
                                        whitespace += c;
                                        escapedSpace += c;
                                        break;

                                    default:
                                        val += val.Length == 0 ? escapedSpace : whitespace;
                                        whitespace = "";
                                        escapedSpace = "";
                                        val += '\\';
                                        val += c;
                                        break;
                                }
                            }
                            else
                            {
                                val += val.Length == 0 ? escapedSpace : whitespace;
                                val += c;
                            }
                            break;

                        case ' ':
                        case '\t':
                        case '\r':
                        case '\n':
                            if (val.Length != 0)
                            {
                                whitespace += c;
                            }
                            break;

                        case '#':
                            finished = true;
                            break;

                        default:
                            val += val.Length == 0 ? escapedSpace : whitespace;
                            whitespace = "";
                            escapedSpace = "";
                            val += c;
                            break;
                    }
                    break;
                }
            }
            if (finished)
            {
                break;
            }
        }
        val += escapedSpace;

        if ((state == ParseStateKey && key.Length != 0) || (state == ParseStateValue && key.Length == 0))
        {
            Util.getProcessLogger().warning("invalid config file entry: \"" + line + "\"");
            return;
        }
        else if (key.Length == 0)
        {
            return;
        }

        setProperty(key, val);
    }

    private void loadConfig()
    {
        string val = getProperty("Ice.Config");
        if (val.Length == 0 || val == "1")
        {
            string? s = Environment.GetEnvironmentVariable("ICE_CONFIG");
            if (s != null && s.Length != 0)
            {
                val = s;
            }
        }

        if (val.Length > 0)
        {
            char[] separator = { ',' };
            string[] files = val.Split(separator);
            for (int i = 0; i < files.Length; i++)
            {
                load(files[i].Trim());
            }

            _properties["Ice.Config"] = new PropertyValue(val, true);
        }
    }

    private readonly Dictionary<string, PropertyValue> _properties = [];
    private readonly object _mutex = new();
}
