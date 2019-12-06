//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Text.RegularExpressions;

namespace Ice
{
    public sealed class Properties
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

        /// <summary>
        /// Get a property by key.
        /// If the property is not set, an empty
        /// string is returned.
        ///
        /// </summary>
        /// <param name="key">The property key.
        ///
        /// </param>
        /// <returns>The property value.
        ///
        /// </returns>
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

        /// <summary>
        /// Get a property by key.
        /// If the property is not set, the
        /// given default value is returned.
        ///
        /// </summary>
        /// <param name="key">The property key.
        ///
        /// </param>
        /// <param name="value">The default value to use if the property does not
        /// exist.
        ///
        /// </param>
        /// <returns>The property value or the default value.
        ///
        /// </returns>
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

        /// <summary>
        /// Get a property as an integer.
        /// If the property is not set, 0
        /// is returned.
        ///
        /// </summary>
        /// <param name="key">The property key.
        ///
        /// </param>
        /// <returns>The property value interpreted as an integer.
        ///
        /// </returns>
        public int getPropertyAsInt(string key)
        {
            return getPropertyAsIntWithDefault(key, 0);
        }

        /// <summary>
        /// Get a property as an integer.
        /// If the property is not set, the
        /// given default value is returned.
        ///
        /// </summary>
        /// <param name="key">The property key.
        ///
        /// </param>
        /// <param name="value">The default value to use if the property does not
        /// exist.
        ///
        /// </param>
        /// <returns>The property value interpreted as an integer, or the
        /// default value.
        ///
        /// </returns>
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

        /// <summary>
        /// Get a property as a list of strings.
        /// The strings must be
        /// separated by whitespace or comma. If the property is not set,
        /// an empty list is returned. The strings in the list can contain
        /// whitespace and commas if they are enclosed in single or double
        /// quotes. If quotes are mismatched, an empty list is returned.
        /// Within single quotes or double quotes, you can escape the
        /// quote in question with \, e.g. O'Reilly can be written as
        /// O'Reilly, "O'Reilly" or 'O\'Reilly'.
        ///
        /// </summary>
        /// <param name="key">The property key.
        ///
        /// </param>
        /// <returns>The property value interpreted as a list of strings.
        ///
        /// </returns>
        public string[] getPropertyAsList(string key)
        {
            return getPropertyAsListWithDefault(key, null);
        }

        /// <summary>
        /// Get a property as a list of strings.
        /// The strings must be
        /// separated by whitespace or comma. If the property is not set,
        /// the default list is returned. The strings in the list can contain
        /// whitespace and commas if they are enclosed in single or double
        /// quotes. If quotes are mismatched, the default list is returned.
        /// Within single quotes or double quotes, you can escape the
        /// quote in question with \, e.g. O'Reilly can be written as
        /// O'Reilly, "O'Reilly" or 'O\'Reilly'.
        ///
        /// </summary>
        /// <param name="key">The property key.
        ///
        /// </param>
        /// <param name="value">The default value to use if the property is not set.
        ///
        /// </param>
        /// <returns>The property value interpreted as list of strings, or the
        /// default value.
        ///
        /// </returns>
        public string[] getPropertyAsListWithDefault(string key, string[] val)
        {
            if (val == null)
            {
                val = Array.Empty<string>();
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

        /// <summary>
        /// Get all properties whose keys begins with
        /// prefix.
        /// If
        /// prefix is an empty string,
        /// then all properties are returned.
        ///
        /// </summary>
        /// <param name="prefix">The prefix to search for (empty string if none).
        /// </param>
        /// <returns>The matching property set.</returns>
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

        /// <summary>
        /// Set a property.
        /// To unset a property, set it to
        /// the empty string.
        ///
        /// </summary>
        /// <param name="key">The property key.
        /// </param>
        /// <param name="value">The property value.
        ///
        /// </param>
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

            //
            // Check if the property is legal.
            //
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
                    bool mismatchCase = false;
                    string otherKey = "";
                    if (!propPrefix.ToUpper().Equals(prefix.ToUpper()))
                    {
                        continue;
                    }

                    bool found = false;
                    foreach (var prop in validProps)
                    {
                        Regex r = new Regex(prop.pattern());
                        Match m = r.Match(key);
                        found = m.Success;
                        if (found)
                        {
                            if (prop.deprecated())
                            {
                                logger.warning("deprecated property: " + key);
                                if (prop.deprecatedBy() != null)
                                {
                                    key = prop.deprecatedBy();
                                }
                            }
                            break;
                        }

                        if (!found)
                        {
                            r = new Regex(prop.pattern().ToUpper());
                            m = r.Match(key.ToUpper());
                            if (m.Success)
                            {
                                found = true;
                                mismatchCase = true;
                                otherKey = prop.pattern().Replace("\\", "").Replace("^", "").Replace("$", "");
                                break;
                            }
                        }
                    }
                    if (!found)
                    {
                        logger.warning("unknown property: " + key);
                    }
                    else if (mismatchCase)
                    {
                        logger.warning("unknown property: `" + key + "'; did you mean `" + otherKey + "'");
                    }
                }
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

        /// <summary>
        /// Get a sequence of command-line options that is equivalent to
        /// this property set.
        /// Each element of the returned sequence is
        /// a command-line option of the form
        /// --key=value.
        ///
        /// </summary>
        /// <returns>The command line options for this property set.</returns>
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

        /// <summary>
        /// Convert a sequence of command-line options into properties.
        /// All options that begin with
        /// --prefix. are
        /// converted into properties. If the prefix is empty, all options
        /// that begin with -- are converted to properties.
        ///
        /// </summary>
        /// <param name="prefix">The property prefix, or an empty string to
        /// convert all options starting with --.
        ///
        /// </param>
        /// <param name="options">The command-line options.
        ///
        /// </param>
        /// <returns>The command-line options that do not start with the specified
        /// prefix, in their original order.</returns>
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
                    if (opt.IndexOf('=') == -1)
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
        /// All options that begin with one of the following prefixes
        /// are converted into properties: --Ice, --IceBox, --IceGrid,
        /// --IceSSL, --IceStorm, and --Glacier2.
        ///
        /// </summary>
        /// <param name="options">The command-line options.
        ///
        /// </param>
        /// <returns>The command-line options that do not start with one of
        /// the listed prefixes, in their original order.</returns>
        public string[] parseIceCommandLineOptions(string[] options)
        {
            string[] args = options;
            foreach (var name in IceInternal.PropertyNames.clPropNames)
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
                using (System.IO.StreamReader sr = new System.IO.StreamReader(file))
                {
                    parse(sr);
                }
            }
            catch (System.IO.IOException ex)
            {
                FileException fe = new FileException(ex);
                fe.path = file;
                throw fe;
            }
        }

        /// <summary>
        /// Create a copy of this property set.
        /// </summary>
        /// <returns>A copy of this property set.</returns>
        public Properties Clone()
        {
            lock (this)
            {
                return new Properties(this);
            }
        }

        public List<string> getUnusedProperties()
        {
            lock (this)
            {
                List<string> unused = new List<string>();
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

        internal Properties(Properties p) : this()
        {
            foreach (KeyValuePair<string, PropertyValue> entry in p._properties)
            {
                _properties[entry.Key] = new PropertyValue(entry.Value);
            }
        }

        internal Properties()
        {
            _properties = new Dictionary<string, PropertyValue>();
        }

        internal Properties(ref string[] args, Properties defaults) : this()
        {
            if (defaults != null)
            {
                foreach (KeyValuePair<string, PropertyValue> entry in defaults._properties)
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
                    if (line.IndexOf('=') == -1)
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
            if (val.Length == 0 || val.Equals("1"))
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

        private Dictionary<string, PropertyValue> _properties;
    }
}
