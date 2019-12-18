//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Linq;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Text.RegularExpressions;

namespace Ice
{
    public sealed partial class Communicator
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
        /// If the property is not set, returns null.
        ///
        /// </summary>
        /// <param name="key">The property key.
        ///
        /// </param>
        /// <returns>The property value.
        ///
        /// </returns>
        public string? GetProperty(string key)
        {
            lock (_properties)
            {
                PropertyValue pv;
                if (_properties.TryGetValue(key, out pv))
                {
                    pv.used = true;
                    return pv.val;
                }
                return null;
            }
        }

        /// <summary>
        /// Get a property as an integer.
        /// If the property is not set, returns null.
        ///
        /// </summary>
        /// <param name="key">The property key.
        ///
        /// </param>
        /// <returns>The property value interpreted as an integer or null.
        ///
        /// </returns>
        public int? GetPropertyAsInt(string key)
        {
            lock (_properties)
            {
                PropertyValue pv;
                if (_properties.TryGetValue(key, out pv))
                {
                    pv.used = true;
                    return int.Parse(pv.val, CultureInfo.InvariantCulture);
                }
                return null;
            }
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
        public string[]? GetPropertyAsList(string key)
        {
            lock (_properties)
            {
                PropertyValue pv;
                if (_properties.TryGetValue(key, out pv))
                {
                    pv.used = true;
                    return IceUtilInternal.StringUtil.splitString(pv.val, ", \t\r\n");
                }
                return null;
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
        /// <param name="forPrefix">The prefix to search for (empty string if none).
        /// </param>
        /// <returns>The matching property set.</returns>
        public Dictionary<string, string> GetProperties(string forPrefix = "")
        {
            lock (_properties)
            {
                Dictionary<string, string> result = new Dictionary<string, string>();

                foreach (string s in _properties.Keys)
                {
                    if (forPrefix.Length == 0 || s.StartsWith(forPrefix, StringComparison.Ordinal))
                    {
                        PropertyValue pv = _properties[s];
                        pv.used = true;
                        result[s] = pv.val;
                    }
                }
                return result;
            }
        }

        public T? GetPropertyAsProxy<T>(string prefix, ProxyFactory<T> factory) where T : class, IObjectPrx
        {
            string? proxy = GetProperty(prefix);
            if (proxy == null)
            {
                return null;
            }
            return factory(CreateReference(proxy, prefix));
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
        public void SetProperty(string key, string value)
        {
            //
            // Trim whitespace
            //
            key = key.Trim();
            if (key.Length == 0)
            {
                throw new ArgumentException("Attempt to set property with empty key", nameof(key));
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
                                string? deprecatedBy = prop.deprecatedBy();
                                if (deprecatedBy != null)
                                {
                                    key = deprecatedBy;
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

            lock (_properties)
            {
                if (value.Length == 0)
                {
                    _properties.Remove(key);
                }
                else
                {
                    PropertyValue pv;
                    if (_properties.TryGetValue(key, out pv))
                    {
                        pv.val = value;
                    }
                    else
                    {
                        pv = new PropertyValue(value, false);
                    }
                    _properties[key] = pv;
                }
            }
        }

        public string? RemoveProperty(string key)
        {
            PropertyValue pv;
            if (_properties.TryGetValue(key, out pv))
            {
                _properties.Remove(key);
                return pv.val;
            }
            return null;
        }

        public List<string> GetUnusedProperties()
        {
            lock (_properties)
            {
                List<string> unused = new List<string>();
                foreach (KeyValuePair<string, PropertyValue> entry in _properties)
                {
                    if (!entry.Value.used && !entry.Key.StartsWith("--Ice.Config"))
                    {
                        unused.Add(entry.Key);
                    }
                }
                return unused;
            }
        }

        private const int ParseStateKey = 0;
        private const int ParseStateValue = 1;

        internal static (string? Key, string value) ParseLine(string line)
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
                return (null, string.Empty);
            }
            else if (key.Length == 0)
            {
                return (null, string.Empty);
            }

            return (key, val);
        }

        internal static Dictionary<string, string> LoadConfig(string iceConfig)
        {
            Dictionary<string, string> properties = new Dictionary<string, string>();
            string[] files = iceConfig.Split(",");
            foreach (var file in files)
            {
                using System.IO.StreamReader input = new System.IO.StreamReader(file.Trim());
                string line;
                while ((line = input.ReadLine()) != null)
                {
                    var result = ParseLine(line);
                    if (result.Key != null)
                    {
                        properties[result.Key] = result.value;
                    }
                }
            }
            return properties;
        }

        private readonly Dictionary<string, PropertyValue> _properties = new Dictionary<string, PropertyValue>();
    }

    public static class PropertiesExtensions
    {
        public static void ParseIceArgs(this Dictionary<string, string> properties, ref string[] args)
        {
            foreach (var name in IceInternal.PropertyNames.clPropNames)
            {
                properties.ParseArgs(ref args, name);
            }
        }

        public static void ParseArgs(this Dictionary<string, string> properties, ref string[] args, string prefix = "")
        {
            if (prefix.Length > 0 && !prefix.EndsWith("."))
            {
                prefix += '.';
            }
            prefix = "--" + prefix;

            if (prefix == "--" || prefix == "--Ice.")
            {
                string? loadConfigFiles = args.FirstOrDefault(arg => arg.StartsWith("--Ice.Config"));
                if (loadConfigFiles != null)
                {
                    if (loadConfigFiles.IndexOf('=') == -1)
                    {
                        loadConfigFiles += "=1";
                    }
                    var result = Communicator.ParseLine(loadConfigFiles.Substring(2));
                    Debug.Assert(result.Key != null);
                    properties[result.Key] = result.value;
                    args = args.Where(arg => !arg.StartsWith("--Ice.Config")).ToArray();
                    foreach (var p in Communicator.LoadConfig(result.value))
                    {
                        properties[p.Key] = p.Value;
                    }
                }
            }

            List<string> unused = new List<string>();
            foreach (var arg in args)
            {
                if (arg.StartsWith(prefix, StringComparison.Ordinal))
                {
                    var r = Communicator.ParseLine((arg.IndexOf('=') == -1 ? $"{arg}=1" : arg).Substring(2));
                    Debug.Assert(r.Key != null);
                    properties[r.Key] = r.value;
                }
                else
                {
                    unused.Add(arg);
                }
            }
            args = unused.ToArray();
        }

        public static void LoadIceConfigFile(this Dictionary<string, string> properties, string configFile)
        {
            foreach (var p in Communicator.LoadConfig(configFile))
            {
                properties[p.Key] = p.Value;
            }
        }
    }
}
