// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Text;
using System.Text.RegularExpressions;

namespace ZeroC.Ice
{
    /// <summary>Provides property-related extension methods for <see cref="Communicator"/> and
    /// <see cref="Dictionary{TKey, TValue}"/> of strings.</summary>
    public static class Properties
    {
        /// <summary>Gets the value of a property as a bool. If the property is not set, returns null.</summary>
        /// <param name="communicator">The communicator holding the properties.</param>
        /// <param name="name">The property name.</param>
        /// <returns>True if the property value is the "1" or "True", false if "0" or "False", or null. Values are
        /// case-insensitive.</returns>
        public static bool? GetPropertyAsBool(this Communicator communicator, string name)
        {
            if (communicator.GetProperty(name) is string value)
            {
                try
                {
                    return value switch
                    {
                        "0" => false,
                        "1" => true,
                        _ => bool.Parse(value)
                    };
                }
                catch (Exception ex)
                {
                    throw new InvalidConfigurationException(
                        $"the value `{value}' of property `{name}' is not a bool", ex);
                }
            }
            return null;
        }

        /// <summary>Gets the value of a property as a size in bytes. If the property is not set, returns null.
        /// The value must be an integer followed immediately by an optional size unit of 'K', 'M' or 'G'.
        /// These correspond to kilobytes, megabytes, or gigabytes, respectively.</summary>
        /// <param name="communicator">The communicator holding the properties.</param>
        /// <param name="name">The property name.</param>
        /// <returns>The property value parsed into an integer representing the number of bytes or null.</returns>
        public static int? GetPropertyAsByteSize(this Communicator communicator, string name)
        {
            if (communicator.GetProperty(name) is string value)
            {
                if (!int.TryParse(value, out int size))
                {
                    try
                    {
                        Match match = Regex.Match(value, @"^([0-9]+)([K|M|G])$");

                        if (!match.Success)
                        {
                            throw new InvalidConfigurationException(
                                $"the value `{value}' of property `{name}' is not a byte size");
                        }

                        int intValue = int.Parse(match.Groups[1].Value, CultureInfo.InvariantCulture);
                        string unit = match.Groups[2].Value;

                        checked
                        {
                            try
                            {
                                size = unit switch
                                {
                                    "K" => 1024 * intValue,
                                    "M" => 1024 * 1024 * intValue,
                                    "G" => 1024 * 1024 * 1024 * intValue,
                                    _ => throw new FormatException($"unknown size unit `{unit}'"),
                                };
                            }
                            catch (OverflowException)
                            {
                                size = int.MaxValue;
                            }
                        }
                    }
                    catch (Exception ex)
                    {
                        throw new InvalidConfigurationException(
                            $"the value `{value}' of property `{name}' is not a byte size", ex);
                    }
                }
                return size;
            }
            return null;
        }

        /// <summary>Gets the value of a property as an enumerated type, the conversion does a case insensitive
        /// comparison of the property value with the enumerators and returns the matching enumerator or throws an
        /// exception if none matches the property value. If the property is not set, returns null.</summary>
        /// <typeparam name="TEnum">An enumeration type.</typeparam>
        /// <param name="communicator">The communicator holding the properties.</param>
        /// <param name="name">The property name.</param>
        /// <exception cref="InvalidConfigurationException">If the property value cannot be converted to one of the
        /// enumeration values.</exception>
        /// <returns>The enumerator value or null if the property was not set.</returns>
        public static TEnum? GetPropertyAsEnum<TEnum>(this Communicator communicator, string name) where TEnum : struct
        {
            if (communicator.GetProperty(name) is string value)
            {
                try
                {
                    if (int.TryParse(value, out int _))
                    {
                        throw new FormatException($"numeric values are not accepted for enum properties");
                    }
                    else
                    {
                        return Enum.Parse<TEnum>(value, ignoreCase: true);
                    }
                }
                catch (Exception ex)
                {
                    throw new InvalidConfigurationException(
                        $"the value `{value}' of property `{name}' does not match any enumerator of {typeof(TEnum)}",
                        ex);
                }
            }
            return null;
        }

        /// <summary>Gets the value of a property as an integer. If the property is not set, returns null.</summary>
        /// <param name="communicator">The communicator holding the properties.</param>
        /// <param name="name">The property name.</param>
        /// <returns>The property value parsed into an integer or null.</returns>
        public static int? GetPropertyAsInt(this Communicator communicator, string name)
        {
            if (communicator.GetProperty(name) is string value)
            {
                try
                {
                    return int.Parse(value, CultureInfo.InvariantCulture);
                }
                catch (Exception ex)
                {
                    throw new InvalidConfigurationException(
                        $"the value `{value}' of property `{name}' is not a 32-bit integer", ex);
                }
            }
            return null;
        }

        /// <summary>Gets the value of a property as an array of strings. If the property is not set, returns null. The
        /// value must contain strings separated by whitespace or comma. These strings can contain whitespace and
        /// commas if they are enclosed in single or double quotes. Within single quotes or double quotes, you can
        /// escape the quote in question with \, e.g. O'Reilly can be written as O'Reilly, "O'Reilly" or 'O\'Reilly'.
        /// </summary>
        /// <param name="communicator">The communicator holding the properties.</param>
        /// <param name="name">The property name.</param>
        /// <returns>The property value parsed into an array of strings or null.</returns>
        public static string[]? GetPropertyAsList(this Communicator communicator, string name)
        {
            if (communicator.GetProperty(name) is string value)
            {
                return StringUtil.SplitString(value, ", \t\r\n");
            }
            return null;
        }

        /// <summary>Gets the value of a property as a proxy. If the property is not set, returns null. For ice1
        /// proxies, this method uses the property name to lookup sub-properties such as Name.InvocationTimeout and
        /// fill-in the corresponding property of new proxy. For ice2 proxies, this method is equivalent to parsing the
        /// property's value.</summary>
        /// <param name="communicator">The communicator holding the properties.</param>
        /// <param name="name">The property name.</param>
        /// <param name="factory">The proxy factory. Use IAPrx.Factory to create IAPrx proxies.</param>
        /// <returns>The property value parsed into a proxy or null.</returns>
        public static T? GetPropertyAsProxy<T>(this Communicator communicator, string name, ProxyFactory<T> factory)
            where T : class, IObjectPrx
        {
            if (communicator.GetProperty(name) is string value)
            {
                try
                {
                    return factory(Reference.Parse(value, communicator, name));
                }
                catch (FormatException ex)
                {
                    throw new InvalidConfigurationException($"the value of property `{name}' is not a proxy", ex);
                }
            }
            return null;
        }

        /// <summary>Gets the value of a property as a TimeSpan. If the property is not set, returns null.
        /// The value must be an integer followed immediately by a time unit of 'ms', 's', 'm', 'h', or 'd'. These
        /// correspond to milliseconds, seconds, minutes, hours, and days, respectively. A value of "infinite" can be
        /// used to specify an infinite duration. </summary>
        /// <param name="communicator">The communicator holding the properties.</param>
        /// <param name="name">The property name.</param>
        /// <returns>The property value parsed into a TimeSpan or null.</returns>
        public static TimeSpan? GetPropertyAsTimeSpan(this Communicator communicator, string name)
        {
            if (communicator.GetProperty(name) is string value)
            {
                try
                {
                    return TimeSpanExtensions.Parse(value);
                }
                catch (Exception ex)
                {
                    throw new InvalidConfigurationException(
                        $"the value `{value}' of property `{name}' is not a TimeSpan",
                        ex);
                }
            }
            return null;
        }

        /// <summary>Loads an Ice configuration file.</summary>
        /// <param name="into">The property dictionary into which the loaded properties are added.</param>
        /// <param name="configFile">The path to the Ice configuration file to load.</param>
        public static void LoadIceConfigFile(this Dictionary<string, string> into, string configFile)
        {
            using var input = new System.IO.StreamReader(configFile.Trim());
            string? line;
            while ((line = input.ReadLine()) != null)
            {
                (string name, string value) = ParseLine(line);
                if (name.Length > 0)
                {
                    into[name] = value;
                }
            }
        }

        /// <summary>Extracts the reserved Ice properties from command-line args.</summary>
        /// <param name="into">The property dictionary into which the properties are added.</param>
        /// <param name="args">The command-line args.</param>
        public static void ParseIceArgs(this Dictionary<string, string> into, ref string[] args)
        {
            foreach (string name in PropertyNames.ClassPropertyNames)
            {
                into.ParseArgs(ref args, name);
            }
        }

        /// <summary>Extracts properties from command-line args.</summary>
        /// <param name="into">The property dictionary into which the parsed properties are added.</param>
        /// <param name="args">The command-line args.</param>
        /// <param name="prefix">Only arguments that start with --prefix are extracted.</param>
        public static void ParseArgs(this Dictionary<string, string> into, ref string[] args, string prefix = "")
        {
            if (prefix.Length > 0 && !prefix.EndsWith(".", StringComparison.InvariantCulture))
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
            foreach (string arg in args)
            {
                if (arg.StartsWith(prefix, StringComparison.Ordinal))
                {
                    (string name, string value) = ParseLine((arg.Contains('=') ? arg : $"{arg}=1").Substring(2));
                    if (name.Length > 0)
                    {
                        parsedArgs[name] = value;
                        continue;
                    }
                }
                remaining.Add(arg);
            }

            if ((prefix == "--" || prefix == "--Ice.") &&
                    parsedArgs.TryGetValue("Ice.Config", out string? configFileList))
            {
                foreach (string file in configFileList.Split(","))
                {
                    into.LoadIceConfigFile(file);
                }
            }

            foreach (KeyValuePair<string, string> p in parsedArgs)
            {
                into[p.Key] = p.Value;
            }

            args = remaining.ToArray();
        }

        internal static void ValidatePropertyName(this Communicator communicator, string name)
        {
            name = name.Trim();
            if (name.Length == 0)
            {
                throw new ArgumentException("attempt to set property with empty key", nameof(name));
            }

            int dotPos = name.IndexOf('.');
            if (dotPos != -1)
            {
                string prefix = name.Substring(0, dotPos);
                foreach (Property[] validProps in PropertyNames.ValidProperties)
                {
                    string pattern = validProps[0].Pattern;
                    dotPos = pattern.IndexOf('.');
                    Debug.Assert(dotPos != -1);
                    string propPrefix = pattern.Substring(1, dotPos - 2);
                    bool mismatchCase = false;
                    string otherKey = "";
                    if (!propPrefix.Equals(prefix, StringComparison.InvariantCultureIgnoreCase))
                    {
                        continue;
                    }

                    bool found = false;
                    foreach (Property prop in validProps)
                    {
                        var r = new Regex(prop.Pattern);
                        Match m = r.Match(name);
                        found = m.Success;
                        if (found)
                        {
                            if (prop.Deprecated)
                            {
                                communicator.Logger.Warning($"deprecated property: `{name}'");
                                string? deprecatedBy = prop.DeprecatedBy;
                                if (deprecatedBy != null)
                                {
                                    name = deprecatedBy;
                                }
                            }
                            break;
                        }

                        if (!found)
                        {
                            r = new Regex(prop.Pattern.ToUpperInvariant());
                            m = r.Match(name.ToUpperInvariant());
                            if (m.Success)
                            {
                                found = true;
                                mismatchCase = true;
                                otherKey = prop.Pattern.Replace("\\", "").Replace("^", "").Replace("$", "");
                                break;
                            }
                        }
                    }
                    if (!found)
                    {
                        communicator.Logger.Warning($"unknown property: `{name}'");
                    }
                    else if (mismatchCase)
                    {
                        communicator.Logger.Warning($"unknown property: `{name}'; did you mean `{otherKey}'");
                    }
                }
            }
        }

        private static (string Name, string Value) ParseLine(string line)
        {
            var name = new StringBuilder();
            var value = new StringBuilder();

            ParseState state = ParseState.Key;

            var whitespace = new StringBuilder();
            var escapedspace = new StringBuilder();
            bool finished = false;
            for (int i = 0; i < line.Length; ++i)
            {
                char c = line[i];
                switch (state)
                {
                    case ParseState.Key:
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
                                            name.Append(whitespace);
                                            whitespace.Clear();
                                            name.Append(c);
                                            break;

                                        case ' ':
                                            if (name.Length != 0)
                                            {
                                                whitespace.Append(c);
                                            }
                                            break;

                                        default:
                                            name.Append(whitespace);
                                            whitespace.Clear();
                                            name.Append('\\');
                                            name.Append(c);
                                            break;
                                    }
                                }
                                else
                                {
                                    name.Append(whitespace);
                                    name.Append(c);
                                }
                                break;

                            case ' ':
                            case '\t':
                            case '\r':
                            case '\n':
                                if (name.Length != 0)
                                {
                                    whitespace.Append(c);
                                }
                                break;

                            case '=':
                                whitespace.Clear();
                                state = ParseState.Value;
                                break;

                            case '#':
                                finished = true;
                                break;

                            default:
                                name.Append(whitespace);
                                whitespace.Clear();
                                name.Append(c);
                                break;
                        }
                        break;
                    }

                    case ParseState.Value:
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
                                            value.Append(value.Length == 0 ? escapedspace : whitespace);
                                            whitespace.Clear();
                                            escapedspace.Clear();
                                            value.Append(c);
                                            break;

                                        case ' ':
                                            whitespace.Append(c);
                                            escapedspace.Append(c);
                                            break;

                                        default:
                                            value.Append(value.Length == 0 ? escapedspace : whitespace);
                                            whitespace.Clear();
                                            escapedspace.Clear();
                                            value.Append('\\');
                                            value.Append(c);
                                            break;
                                    }
                                }
                                else
                                {
                                    value.Append(value.Length == 0 ? escapedspace : whitespace);
                                    value.Append(c);
                                }
                                break;

                            case ' ':
                            case '\t':
                            case '\r':
                            case '\n':
                                if (value.Length != 0)
                                {
                                    whitespace.Append(c);
                                }
                                break;

                            case '#':
                                finished = true;
                                break;

                            default:
                                value.Append(value.Length == 0 ? escapedspace : whitespace);
                                whitespace.Clear();
                                escapedspace.Clear();
                                value.Append(c);
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
            value.Append(escapedspace);

            if ((state == ParseState.Key && name.Length != 0) || (state == ParseState.Value && name.Length == 0))
            {
                throw new FormatException($"invalid config file entry: \"{line}\"");
            }

            return (name.ToString(), value.ToString());
        }

        private enum ParseState : byte
        {
            Key,
            Value
        }
    }

    /// <summary>Helper struct used for property name validation.</summary>
    internal readonly struct Property
    {
        internal bool Deprecated { get; }
        internal string? DeprecatedBy { get; }
        internal string Pattern { get; }

        internal Property(string pattern, bool deprecated = false, string? deprecatedBy = null)
        {
            Pattern = pattern;
            Deprecated = deprecated;
            DeprecatedBy = deprecatedBy;
        }
    }
}
