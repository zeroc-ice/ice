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
    public sealed partial class Communicator
    {
        private readonly Dictionary<string, PropertyValue> _properties = new Dictionary<string, PropertyValue>();

        private class PropertyValue
        {
            internal string Val;
            internal bool Used;

            internal PropertyValue(string v, bool u)
            {
                Val = v;
                Used = u;
            }
        }

        /// <summary>Get the value of a property. If the property is not set, returns null.</summary>
        /// <param name="key">The property key.</param>
        /// <returns>The property value.</returns>
        public string? GetProperty(string key)
        {
            lock (_properties)
            {
                if (_properties.TryGetValue(key, out var pv))
                {
                    pv.Used = true;
                    return pv.Val;
                }
                return null;
            }
        }

        /// <summary>Get the value of a property as an integer. If the property is not set, returns null.</summary>
        /// <param name="key">The property key.</param>
        /// <returns>The property value parsed into an integer or null.</returns>
        public int? GetPropertyAsInt(string key)
        {
            lock (_properties)
            {
                if (_properties.TryGetValue(key, out var pv))
                {
                    pv.Used = true;
                    return int.Parse(pv.Val, CultureInfo.InvariantCulture);
                }
                return null;
            }
        }

        /// <summary>
        /// Get the value of a property as an array of strings. If the property is not set, returns null.
        /// The value must contain strings separated by whitespace or comma. These strings can contain
        /// whitespace and commas if they are enclosed in single or double quotes. Within single quotes or
        /// double quotes, you can escape the quote in question with \, e.g. O'Reilly can be written as
        /// O'Reilly, "O'Reilly" or 'O\'Reilly'.
        /// </summary>
        /// <param name="key">The property key.</param>
        /// <returns>The property value parsed into an array of strings or null.</returns>
        public string[]? GetPropertyAsList(string key)
        {
            lock (_properties)
            {
                if (_properties.TryGetValue(key, out var pv))
                {
                    pv.Used = true;
                    return IceUtilInternal.StringUtil.splitString(pv.Val, ", \t\r\n");
                }
                return null;
            }
        }

        /// <summary>Get all properties whose keys begins with forPrefix.
        /// If forPrefix is an empty string, then all properties are returned.
        /// </summary>
        /// <param name="forPrefix">The prefix to search for (empty string if none).</param>
        /// <returns>The matching property set.</returns>
        public Dictionary<string, string> GetProperties(string forPrefix = "")
        {
            lock (_properties)
            {
                var result = new Dictionary<string, string>();

                foreach (string s in _properties.Keys)
                {
                    if (forPrefix.Length == 0 || s.StartsWith(forPrefix, StringComparison.Ordinal))
                    {
                        PropertyValue pv = _properties[s];
                        pv.Used = true;
                        result[s] = pv.Val;
                    }
                }
                return result;
            }
        }

        /// <summary>Get the value of a property as a proxy. If the property is not set, returns null.</summary>
        /// <param name="key">The property key. This key is also used as a prefix for proxy options.</param>
        /// <param name="factory">The proxy factory. Use IAPrx.Factory to create IAPrx proxies.</param>
        /// <returns>The property value parsed into a proxy or null.</returns>
        public T? GetPropertyAsProxy<T>(string key, ProxyFactory<T> factory) where T : class, IObjectPrx
        {
            string? proxy = GetProperty(key);
            if (proxy == null)
            {
                return null;
            }
            return factory(CreateReference(proxy, key));
        }

        /// <summary>Insert a new property or change the value of an existing property.
        /// Setting the value of a property to the empty string removes this property
        /// if it was present, and does nothing otherwise.</summary>
        /// <param name="key">The property key.</param>
        /// <param name="value">The property value.</param>
        public void SetProperty(string key, string value)
        {
            ValidateProperty(key, value);

            lock (_properties)
            {
                SetPropertyImpl(key, value);
            }
        }

        /// <summary>Insert new properties or change the value of existing properties.
        /// Setting the value of a property to the empty string removes this property
        /// if it was present, and does nothing otherwise.</summary>
        /// <param name="updates">A dictionary that contains the new, updated and removed properties.</param>
        public void SetProperties(Dictionary<string, string> updates)
        {
            foreach (var entry in updates)
            {
                ValidateProperty(entry.Key, entry.Value);
            }

            lock (_properties)
            {
                foreach (var entry in updates)
                {
                    SetPropertyImpl(entry.Key, entry.Value);
                }
            }
        }

        /// <summary>Remove a property.</summary>
        /// <param name="key">The property key.</param>
        /// <returns>The property value or null if this property was not set.</returns>
        public string? RemoveProperty(string key)
        {
            if (_properties.TryGetValue(key, out var pv))
            {
                _properties.Remove(key);
                return pv.Val;
            }
            return null;
        }

        /// <summary>Get all properties that were not read.</summary>
        /// <returns>The properties that were not read as a list of keys.</returns>
        public List<string> GetUnusedProperties()
        {
            lock (_properties)
            {
                List<string> unused = new List<string>();
                foreach (KeyValuePair<string, PropertyValue> entry in _properties)
                {
                    if (!entry.Value.Used)
                    {
                        unused.Add(entry.Key);
                    }
                }
                return unused;
            }
        }

        private void SetPropertyImpl(string key, string value)
        {
            // Must be called with validated a validated property and with _properties locked

            key = key.Trim();
            Debug.Assert(key.Length > 0);
            if (value.Length == 0)
            {
                _properties.Remove(key);
            }
            else
            {
                // These properties are always marked "used"
                bool used = (key == "Ice.ConfigFile" || key == "Ice.ProgramName");

                if (_properties.TryGetValue(key, out var pv))
                {
                    pv.Val = value;
                    pv.Used = used;
                }
                else
                {
                    _properties[key] = new PropertyValue(value, used);
                }
            }
        }

        private void ValidateProperty(string key, string value)
        {
            key = key.Trim();
            if (key.Length == 0)
            {
                throw new ArgumentException("Attempt to set property with empty key", nameof(key));
            }

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
                                _logger.warning("deprecated property: " + key);
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
                        _logger.warning("unknown property: " + key);
                    }
                    else if (mismatchCase)
                    {
                        _logger.warning("unknown property: `" + key + "'; did you mean `" + otherKey + "'");
                    }
                }
            }
        }
    }

    public static class PropertiesExtensions
    {
        /// <summary>Extract the reserved Ice properties from command-line args.</summary>
        /// <param name="into">The property dictionary into which the properties are added.</param>
        /// <param name="args">The command-line args.</param>
        public static void ParseIceArgs(this Dictionary<string, string> into, ref string[] args)
        {
            foreach (var name in IceInternal.PropertyNames.clPropNames)
            {
                into.ParseArgs(ref args, name);
            }
        }

        /// <summary>Extract properties from command-line args.</summary>
        /// <param name="into">The property dictionary into which the parsed properties are added.</param>
        /// <param name="args">The command-line args.</param>
        /// <param name="prefix">Only arguments that start with --prefix are extracted.</param>
        public static void ParseArgs(this Dictionary<string, string> into, ref string[] args, string prefix = "")
        {
            if (prefix.Length > 0 && !prefix.EndsWith("."))
            {
                prefix += '.';
            }
            prefix = "--" + prefix;

            if ((prefix == "--" || prefix == "--Ice.") && Array.Find(args, arg => arg == "--Ice.Config") != null)
            {
                throw new ArgumentException("--Ice.Config requires a value", nameof(args));
            }

            var remaining = new List<string>();
            var parsedArgs = new Dictionary<string, string>();
            foreach (var arg in args)
            {
                if (arg.StartsWith(prefix, StringComparison.Ordinal))
                {
                    var r = ParseLine((arg.IndexOf('=') == -1 ? $"{arg}=1" : arg).Substring(2));
                    if (r.Name.Length > 0)
                    {
                        parsedArgs[r.Name] = r.Value;
                        continue;
                    }
                }
                remaining.Add(arg);
            }

            if ((prefix == "--" || prefix == "--Ice.") &&
                    parsedArgs.TryGetValue("Ice.Config", out string configFileList))
            {
                foreach (var file in configFileList.Split(","))
                {
                    into.LoadIceConfigFile(file);
                }
            }

            foreach (var p in parsedArgs)
            {
                into[p.Key] = p.Value;
            }

            args = remaining.ToArray();
        }

        /// <summary>Load Ice configuration file.</summary>
        /// <param name="into">The property dictionary into which the loaded properties are added.</param>
        /// <param name="configFile">The path to the Ice configuration file to load.</param>
        public static void LoadIceConfigFile(this Dictionary<string, string> into, string configFile)
        {
            using System.IO.StreamReader input = new System.IO.StreamReader(configFile.Trim());
            string line;
            while ((line = input.ReadLine()) != null)
            {
                var result = ParseLine(line);
                if (result.Name.Length > 0)
                {
                    into[result.Name] = result.Value;
                }
            }
        }

        private static (string Name, string Value) ParseLine(string line)
        {
            const bool ParseStateKey = false;
            const bool ParseStateValue = true;

            string key = "";
            string val = "";

            bool state = ParseStateKey;

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
                throw new FormatException($"invalid config file entry: \"{line}\"");
            }

            return (key, val);
        }
    }
}
