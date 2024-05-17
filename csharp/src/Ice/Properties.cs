// Copyright (c) ZeroC, Inc.

#nullable enable

using Ice.Internal;
using System.Diagnostics;
using System.Globalization;
using System.Text.RegularExpressions;

namespace Ice;

/// <summary>
// A property set used to configure Ice and Ice applications. Properties are key/value pairs, with both keys and
// values being strings.
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
    /// Creates a new empty property set.
    /// </summary>
    public Properties() { }

    /// <summary>
    /// Creates a property set initialized from an argument vector.
    /// </summary>
    /// <param name="args">A command-line argument vector, possibly containing
    /// options to set properties. If the command-line options include
    /// a --Ice.Config option, the corresponding configuration
    /// files are parsed. If the same property is set in a configuration
    /// file and in the argument vector, the argument vector takes precedence.
    /// This method modifies the argument vector by removing any Ice-related options.</param>
    /// <param name="defaults">Default values for the property set. Settings in configuration
    /// files and args override these defaults. May be null.</param>
    /// <returns>A property set initialized with the property settings
    /// that were removed from args.</returns>
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
                if (!line.Contains('='))
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
    ///  <param name="key">The property key.
    ///  </param>
    /// <returns>The property value.
    ///  </returns>
    public string getProperty(string key)
    {
        lock (this)
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
    ///  <param name="key">The property key.
    ///  </param>
    /// <returns>The property value or the default value.
    ///  </returns>
    public string getIceProperty(string key) => getPropertyWithDefault(key, getDefaultProperty(key));

    /// <summary>
    /// Get a property by key.
    /// If the property is not set, the given default value is returned.
    /// </summary>
    ///  <param name="key">The property key.
    ///  </param>
    /// <param name="value">The default value to use if the property does not exist.
    ///  </param>
    /// <returns>The property value or the default value.
    ///  </returns>
    public string getPropertyWithDefault(string key, string val)
    {
        lock (this)
        {
            string result = val;
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
    ///  <param name="key">The property key.
    ///  </param>
    /// <returns>The property value interpreted as an integer.
    ///  </returns>
    public int getPropertyAsInt(string key) => getPropertyAsIntWithDefault(key, 0);

    /// <summary>
    /// Get an Ice property as an integer.
    /// If the property is not set, its default value is returned.
    /// </summary>
    ///  <param name="key">The property key.
    ///  </param>
    /// <returns>The property value interpreted as an integer, or the default value.
    ///  </returns>
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
    ///  <param name="key">The property key.
    ///  </param>
    /// <param name="value">The default value to use if the property does not exist.
    ///  </param>
    /// <returns>The property value interpreted as an integer, or the default value.
    ///  </returns>
    public int getPropertyAsIntWithDefault(string key, int val)
    {
        lock (this)
        {
            if (!_properties.TryGetValue(key, out PropertyValue? pv))
            {
                return val;
            }
            pv.used = true;
            try
            {
                return int.Parse(pv.value, CultureInfo.InvariantCulture);
            }
            catch (FormatException)
            {
                Util.getProcessLogger().warning("numeric property " + key +
                                                " set to non-numeric value, defaulting to " + val);
                return val;
            }
        }
    }

    /// <summary>
    /// Get a property as a list of strings.
    /// The strings must be separated by whitespace or comma. If the property is
    ///  not set, an empty list is returned. The strings in the list can contain whitespace and commas if they are
    ///  enclosed in single or double quotes. If quotes are mismatched, an empty list is returned. Within single quotes
    ///  or double quotes, you can escape the quote in question with a backslash, e.g. O'Reilly can be written as
    ///  O'Reilly, "O'Reilly" or 'O\'Reilly'.
    /// </summary>
    ///  <param name="key">The property key.
    ///  </param>
    /// <returns>The property value interpreted as a list of strings.
    ///  </returns>
    public string[] getPropertyAsList(string key) => getPropertyAsListWithDefault(key, []);

    /// <summary>
    /// Get an Ice property as a list of strings.
    /// The strings must be separated by whitespace or comma. If the property is
    ///  not set, its default list is returned. The strings in the list can contain whitespace and commas if they are
    ///  enclosed in single or double quotes. If quotes are mismatched, the default list is returned. Within single
    ///  quotes or double quotes, you can escape the quote in question with a backslash, e.g. O'Reilly can be written as
    ///  O'Reilly, "O'Reilly" or 'O\'Reilly'.
    /// </summary>
    ///  <param name="key">The property key.
    ///  </param>
    /// <param name="value">The default value to use if the property is not set.
    ///  </param>
    /// <returns>The property value interpreted as list of strings, or the default value.
    ///  </returns>
    public string[] getIcePropertyAsList(string key)
    {
        string[] defaultList = UtilInternal.StringUtil.splitString(getDefaultProperty(key), ", \t\r\n");
        return getPropertyAsListWithDefault(key, defaultList);
    }

    /// <summary>
    /// Get a property as a list of strings.
    /// The strings must be separated by whitespace or comma. If the property is
    ///  not set, the default list is returned. The strings in the list can contain whitespace and commas if they are
    ///  enclosed in single or double quotes. If quotes are mismatched, the default list is returned. Within single
    ///  quotes or double quotes, you can escape the quote in question with a backslash, e.g. O'Reilly can be written as
    ///  O'Reilly, "O'Reilly" or 'O\'Reilly'.
    /// </summary>
    ///  <param name="key">The property key.
    ///  </param>
    /// <param name="value">The default value to use if the property is not set.
    ///  </param>
    /// <returns>The property value interpreted as list of strings, or the default value.
    ///  </returns>
    public string[] getPropertyAsListWithDefault(string key, string[] val)
    {
        val ??= [];

        lock (this)
        {
            if (!_properties.TryGetValue(key, out PropertyValue? pv))
            {
                return val;
            }

            pv.used = true;

            string[] result = Ice.UtilInternal.StringUtil.splitString(pv.value, ", \t\r\n");
            if (result == null)
            {
                Util.getProcessLogger().warning("mismatched quotes in property " + key
                                                + "'s value, returning default value");
                return val;
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
    ///  properties are returned.
    /// </summary>
    ///  <param name="prefix">The prefix to search for (empty string if none).
    ///  </param>
    /// <returns>The matching property set.</returns>
    public Dictionary<string, string> getPropertiesForPrefix(string prefix)
    {
        lock (this)
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
    ///  <param name="key">The property key.
    ///  </param>
    /// <param name="value">The property value.
    ///  </param>
    public void setProperty(string key, string val)
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

        // Finds the corresponding Ice property if it exists. Also logs warnings for unknown Ice properties and
        // case-insensitive Ice property prefix matches.
        Property? prop = findProperty(key, true);

        // If the property is deprecated, log a warning.
        if (prop is not null && prop.deprecated)
        {
            Util.getProcessLogger().warning("setting deprecated property: " + key);
        }

        lock (this)
        {
            // Set or clear the property.
            if (val != null && val.Length > 0)
            {
                if (_properties.TryGetValue(key, out PropertyValue? pv))
                {
                    pv.value = val;
                }
                else
                {
                    pv = new PropertyValue(val, false);
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
    ///  sequence is a command-line option of the form --key=value.
    /// </summary>
    ///  <returns>The command line options for this property set.</returns>
    public string[] getCommandLineOptions()
    {
        lock (this)
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
    ///  --prefix. are converted into properties. If the prefix is empty, all options that begin with
    ///  -- are converted to properties.
    /// </summary>
    ///  <param name="prefix">The property prefix, or an empty string to convert all options starting with --.
    ///  </param>
    /// <param name="options">The command-line options.
    ///  </param>
    /// <returns>The command-line options that do not start with the specified prefix, in their original order.</returns>
    public string[] parseCommandLineOptions(string pfx, string[] options)
    {
        if (pfx.Length > 0 && pfx[^1] != '.')
        {
            pfx += '.';
        }
        pfx = "--" + pfx;

        List<string> result = [];
        for (int i = 0; i < options.Length; i++)
        {
            string opt = options[i];
            if (opt.StartsWith(pfx, StringComparison.Ordinal))
            {
                if (!opt.Contains('='))
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
    ///  prefixes are converted into properties: --Ice, --IceBox, --IceGrid,
    ///  --IceSSL, --IceStorm, --Freeze, and --Glacier2.
    /// </summary>
    ///  <param name="options">The command-line options.
    ///  </param>
    /// <returns>The command-line options that do not start with one of the listed prefixes, in their original order.</returns>
    public string[] parseIceCommandLineOptions(string[] options)
    {
        string[] args = options;
        foreach (string? name in PropertyNames.clPropNames)
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
            var fe = new FileException(ex);
            fe.path = file;
            throw fe;
        }
    }

    /// <summary>
    /// Create a copy of this property set.
    /// </summary>
    /// <returns>A copy of this property set.</returns>
    public Properties ice_clone_()
    {
        var clonedProperties = new Properties();
        lock (this)
        {
            foreach ((string key, PropertyValue value) in _properties)
            {
                clonedProperties._properties[key] = value.Clone();
            }
        }
        return clonedProperties;
    }

    public List<string> getUnusedProperties()
    {
        lock (this)
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

    /// <summary>
    ///  Find a property in the Ice property set.
    /// </summary>
    /// <param name="key">The property's key.</param>
    /// <param name="logWarnings">Whether to log relevant warnings.</param>
    /// <returns>The found property</returns>
    private static Property? findProperty(string key, bool logWarnings)
    {
        // Check if the property is a known Ice property and log warnings if necessary
        Logger logger = Util.getProcessLogger();
        int dotPos = key.IndexOf('.');

        // If the key doesn't contain a dot, it's not a valid Ice property
        if (dotPos == -1)
        {
            return null;
        }

        string prefix = key.Substring(0, dotPos);

        Property[]? propertyArray = null;

        // Search for the property list that matches the prefix
        foreach (Property[]? validProps in PropertyNames.validProps)
        {
            string pattern = validProps[0].pattern;
            dotPos = pattern.IndexOf('.');

            // Each top level prefix describes a non-empty
            // namespace. Having a string without a prefix followed by a
            // dot is an error.
            Debug.Assert(dotPos != -1);

            // Trim any leading/trailing ^, $, or \ characters from the prefix
            string propPrefix = pattern.Substring(0, dotPos).TrimStart(['^', '$', '\\']);

            if (propPrefix == prefix)
            {
                // We found the property list that matches the prefix
                propertyArray = validProps;
                break;
            }

            // As a courtesy to the user, perform a case-insensitive match and suggest the correct property.
            // Otherwise no other warning is issued.
            if (logWarnings && propPrefix.ToUpper().Equals(prefix.ToUpper()))
            {
                logger.warning("unknown property: `" + key + "'; did you mean `" + propPrefix + "'?");
                return null;
            }
        }

        if (propertyArray == null)
        {
            // The prefix is not a valid Ice property
            return null;
        }

        foreach (var prop in propertyArray)
        {
            if (prop.usesRegex ? Regex.IsMatch(key, prop.pattern) : key == prop.pattern)
            {
                return prop;
            }
        }

        // If we get here, the prefix is valid but the property is unknown
        if (logWarnings)
        {
            logger.warning("unknown property: " + key);
        }
        return null;
    }

    /// <summary>
    /// Gets the default value for a given Ice property.
    /// </summary>
    /// <param name="key">The Ice property key</param>
    /// <returns>The default property value, or an empty string the default is unspecified.</returns>
    /// <exception cref="ArgumentException"></exception>
    private static string getDefaultProperty(string key)
    {
        // Find the property, don't log any warnings.
        Property? prop = findProperty(key, false) ?? throw new ArgumentException("unknown Ice property: " + key);
        return prop.defaultValue;
    }

    private readonly Dictionary<string, PropertyValue> _properties = [];
}
