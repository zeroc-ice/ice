// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using System.Globalization;
using System.Text.RegularExpressions;

namespace Ice;

internal sealed class PropertiesI : Properties
{
    private class PropertyValue
    {
        public PropertyValue(PropertyValue v)
        {
            val = v.val;
            used = v.used;
        }

        public PropertyValue(string v, bool u)
        {
            val = v;
            used = u;
        }

        public string val;
        public bool used;
    }

    public string getProperty(string key)
    {
        lock (this)
        {
            string result = "";
            PropertyValue pv;
            if (_properties.TryGetValue(key, out pv))
            {
                pv.used = true;
                result = pv.val;
            }
            return result;
        }
    }

    public string getIceProperty(string key)
    {
        return getPropertyWithDefault(key, getDefaultProperty(key));
    }

    public string getPropertyWithDefault(string key, string val)
    {
        lock (this)
        {
            string result = val;
            PropertyValue pv;
            if (_properties.TryGetValue(key, out pv))
            {
                pv.used = true;
                result = pv.val;
            }
            return result;
        }
    }

    public int getPropertyAsInt(string key)
    {
        return getPropertyAsIntWithDefault(key, 0);
    }

    public int getIcePropertyAsInt(string key)
    {
        string defaultValueString = getDefaultProperty(key);
        int defaultValue = 0;
        if (!string.IsNullOrEmpty(defaultValueString))
        {
            defaultValue = int.Parse(defaultValueString, CultureInfo.InvariantCulture);
        }

        return getPropertyAsIntWithDefault(key, defaultValue);
    }

    public int getPropertyAsIntWithDefault(string key, int val)
    {
        lock (this)
        {
            PropertyValue pv;
            if (!_properties.TryGetValue(key, out pv))
            {
                return val;
            }
            pv.used = true;
            try
            {
                return int.Parse(pv.val, CultureInfo.InvariantCulture);
            }
            catch (FormatException)
            {
                Util.getProcessLogger().warning("numeric property " + key +
                                                " set to non-numeric value, defaulting to " + val);
                return val;
            }
        }
    }

    public string[] getPropertyAsList(string key)
    {
        return getPropertyAsListWithDefault(key, null);
    }

    public string[] getIcePropertyAsList(string key)
    {
        string[] defaultList = IceUtilInternal.StringUtil.splitString(getDefaultProperty(key), ", \t\r\n");
        return getPropertyAsListWithDefault(key, defaultList);
    }

    public string[] getPropertyAsListWithDefault(string key, string[] val)
    {
        if (val == null)
        {
            val = [];
        }

        lock (this)
        {
            PropertyValue pv;
            if (!_properties.TryGetValue(key, out pv))
            {
                return val;
            }

            pv.used = true;

            string[] result = IceUtilInternal.StringUtil.splitString(pv.val, ", \t\r\n");
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

    public Dictionary<string, string> getPropertiesForPrefix(string prefix)
    {
        lock (this)
        {
            Dictionary<string, string> result = new Dictionary<string, string>();

            foreach (string s in _properties.Keys)
            {
                if (prefix.Length == 0 || s.StartsWith(prefix, StringComparison.Ordinal))
                {
                    PropertyValue pv = _properties[s];
                    pv.used = true;
                    result[s] = pv.val;
                }
            }
            return result;
        }
    }

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

        // Find the property, log warnings if necessary
        var prop = findProperty(key, true);

        // If the property is deprecated by another property, use the new property key
        if (prop != null && prop.deprecatedBy() != null)
        {
            key = prop.deprecatedBy();
        }

        lock (this)
        {
            //
            //
            // Set or clear the property.
            //
            if (val != null && val.Length > 0)
            {
                PropertyValue pv;
                if (_properties.TryGetValue(key, out pv))
                {
                    pv.val = val;
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

    public string[] getCommandLineOptions()
    {
        lock (this)
        {
            string[] result = new string[_properties.Count];
            int i = 0;
            foreach (KeyValuePair<string, PropertyValue> entry in _properties)
            {
                result[i++] = "--" + entry.Key + "=" + entry.Value.val;
            }
            return result;
        }
    }

    public string[] parseCommandLineOptions(string pfx, string[] options)
    {
        if (pfx.Length > 0 && pfx[pfx.Length - 1] != '.')
        {
            pfx += '.';
        }
        pfx = "--" + pfx;

        List<string> result = new List<string>();
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

    public string[] parseIceCommandLineOptions(string[] options)
    {
        string[] args = options;
        foreach (var name in IceInternal.PropertyNames.clPropNames)
        {
            args = parseCommandLineOptions(name, args);
        }
        return args;
    }

    public void load(string file)
    {
        try
        {
            using StreamReader sr = new StreamReader(file);
            parse(sr);
        }
        catch (IOException ex)
        {
            var fe = new FileException(ex);
            fe.path = file;
            throw fe;
        }
    }

    public Properties ice_clone_()
    {
        lock (this)
        {
            return new PropertiesI(this);
        }
    }

    public List<string> getUnusedProperties()
    {
        lock (this)
        {
            var unused = new List<string>();
            foreach (KeyValuePair<string, PropertyValue> entry in _properties)
            {
                if (!entry.Value.used)
                {
                    unused.Add(entry.Key);
                }
            }
            return unused;
        }
    }

    internal PropertiesI(PropertiesI p) : this()
    {
        foreach (KeyValuePair<string, PropertyValue> entry in p._properties)
        {
            _properties[entry.Key] = new PropertyValue(entry.Value);
        }
    }

    internal PropertiesI()
    {
        _properties = new Dictionary<string, PropertyValue>();
    }

    internal PropertiesI(ref string[] args, Properties defaults) : this()
    {
        if (defaults != null)
        {
            foreach (KeyValuePair<string, PropertyValue> entry in ((PropertiesI)defaults)._properties)
            {
                _properties[entry.Key] = new PropertyValue(entry.Value);
            }
        }

        PropertyValue pv;
        if (_properties.TryGetValue("Ice.ProgramName", out pv))
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

    private void parse(System.IO.StreamReader input)
    {
        try
        {
            string line;
            while ((line = input.ReadLine()) != null)
            {
                parseLine(line);
            }
        }
        catch (System.IO.IOException ex)
        {
            SyscallException se = new SyscallException(ex);
            throw se;
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
        string escapedspace = "";
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
                                            val += val.Length == 0 ? escapedspace : whitespace;
                                            whitespace = "";
                                            escapedspace = "";
                                            val += c;
                                            break;

                                        case ' ':
                                            whitespace += c;
                                            escapedspace += c;
                                            break;

                                        default:
                                            val += val.Length == 0 ? escapedspace : whitespace;
                                            whitespace = "";
                                            escapedspace = "";
                                            val += '\\';
                                            val += c;
                                            break;
                                    }
                                }
                                else
                                {
                                    val += val.Length == 0 ? escapedspace : whitespace;
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
                                val += val.Length == 0 ? escapedspace : whitespace;
                                whitespace = "";
                                escapedspace = "";
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
        val += escapedspace;

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
            string s = Environment.GetEnvironmentVariable("ICE_CONFIG");
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
    private static IceInternal.Property findProperty(string key, bool logWarnings)
    {
        // Check if the property is a known Ice property and log warnings if necessary
        Logger logger = Util.getProcessLogger();
        int dotPos = key.IndexOf('.');
        if (dotPos != -1)
        {
            string prefix = key.Substring(0, dotPos);
            foreach (var validProps in IceInternal.PropertyNames.validProps)
            {
                string pattern = validProps[0].pattern();
                dotPos = pattern.IndexOf('.');
                Debug.Assert(dotPos != -1);
                string propPrefix = pattern.Substring(1, dotPos - 2);

                // If the prefix is not the same, skip to the next set of properties
                if (!propPrefix.ToUpper().Equals(prefix.ToUpper()))
                {
                    continue;
                }

                foreach (var prop in validProps)
                {
                    Regex r = new Regex(prop.pattern());
                    Match m = r.Match(key);
                    if (m.Success)
                    {
                        if (prop.deprecated() && logWarnings)
                        {
                            logger.warning("deprecated property: " + key);
                        }
                        return prop;
                    }

                    // Check for case-insensitive match
                    r = new Regex(prop.pattern().ToUpper());
                    m = r.Match(key.ToUpper());
                    if (m.Success)
                    {
                        if (logWarnings)
                        {
                            string otherKey = prop.pattern().Replace("\\", "").Replace("^", "").Replace("$", "");
                            logger.warning("unknown property: `" + key + "'; did you mean `" + otherKey + "'");
                        }
                        return null;
                    }
                }

                // If we get here, the prefix is valid but the property is unknown
                if (logWarnings)
                {
                    logger.warning("unknown property: " + key);
                }
                return null;
            }
        }

        // The key does not match a known Ice property
        return null;
    }

    /// <summary>
    /// Gets the default value for a given Ice property.
    /// </summary>
    /// <param name="key">The Ice property key</param>
    /// <returns>The default property value, or an empty string the default is unspecified.</returns>
    /// <exception cref="ArgumentException"></exception>
    static private string getDefaultProperty(string key)
    {
        // Find the property, don't log any warnings.
        var prop = findProperty(key, false);
        if (prop == null)
        {
            throw new ArgumentException("unknown ice property: " + key);
        }
        return prop.defaultValue();
    }

    private Dictionary<string, PropertyValue> _properties;
}
