// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;

namespace ZeroC.Ice
{
    public sealed partial class Communicator
    {
        private readonly Dictionary<string, PropertyValue> _properties = new();

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

        /// <summary>Gets the value of a property. If the property is not set, returns null.</summary>
        /// <param name="name">The property name.</param>
        /// <returns>The property value.</returns>
        public string? GetProperty(string name)
        {
            lock (_mutex)
            {
                if (_properties.TryGetValue(name, out PropertyValue? pv))
                {
                    pv.Used = true;
                    return pv.Val;
                }
                return null;
            }
        }

        /// <summary>Gets all properties whose keys begins with forPrefix. If forPrefix is the empty string, then all
        /// properties are returned.</summary>
        /// <param name="forPrefix">The prefix to search for (empty string if none).</param>
        /// <returns>The matching property set.</returns>
        public Dictionary<string, string> GetProperties(string forPrefix = "")
        {
            lock (_mutex)
            {
                var result = new Dictionary<string, string>();

                foreach (string name in _properties.Keys)
                {
                    if (forPrefix.Length == 0 || name.StartsWith(forPrefix, StringComparison.Ordinal))
                    {
                        PropertyValue pv = _properties[name];
                        pv.Used = true;
                        result[name] = pv.Val;
                    }
                }
                return result;
            }
        }

        /// <summary>Inserts a new property or changes the value of an existing property. Setting the value of a
        /// property to the empty string removes this property if it was present, and does nothing otherwise.</summary>
        /// <param name="name">The property name.</param>
        /// <param name="value">The property value.</param>
        public void SetProperty(string name, string value)
        {
            this.ValidatePropertyName(name);

            lock (_mutex)
            {
                _ = SetPropertyImpl(name, value);
            }
        }

        /// <summary>Inserts new properties or changes the value of existing properties. Setting the value of a property
        /// to the empty string removes this property if it was present, and does nothing otherwise.</summary>
        /// <param name="updates">A dictionary of properties. This methods removes properties that did not change
        /// anything from this dictionary.</param>
        public void SetProperties(Dictionary<string, string> updates)
        {
            foreach (KeyValuePair<string, string> entry in updates)
            {
                this.ValidatePropertyName(entry.Key);
            }

            lock (_mutex)
            {
                foreach (KeyValuePair<string, string> entry in updates)
                {
                    if (!SetPropertyImpl(entry.Key, entry.Value))
                    {
                        // Starting with .NET 3.0, Remove does not invalidate enumerators
                        updates.Remove(entry.Key);
                    }
                }
            }
        }

        /// <summary>Removes a property.</summary>
        /// <param name="name">The property name.</param>
        /// <returns>true if the property is successfully found and removed; otherwise false.</returns>
        public bool RemoveProperty(string name)
        {
            lock (_mutex)
            {
                return _properties.Remove(name);
            }
        }

        /// <summary>Gets all properties that were not read.</summary>
        /// <returns>The properties that were not read as a list of keys.</returns>
        public List<string> GetUnusedProperties()
        {
            lock (_mutex)
            {
                return _properties.Where(p => !p.Value.Used).Select(p => p.Key).ToList();
            }
        }

        // SetPropertyImpl sets a property and returns true when the property was added, changed or removed, and false
        // otherwise.
        private bool SetPropertyImpl(string name, string value)
        {
            // Must be called with a validated property and with _mutex locked

            name = name.Trim();
            Debug.Assert(name.Length > 0);
            if (value.Length == 0)
            {
                return _properties.Remove(name);
            }
            else if (_properties.TryGetValue(name, out PropertyValue? pv))
            {
                if (pv.Val != value)
                {
                    pv.Val = value;
                    return true;
                }
                // else Val == value, nothing to do
            }
            else
            {
                // These properties are always marked "used"
                bool used = name == "Ice.ConfigFile" || name == "Ice.ProgramName";
                _properties[name] = new PropertyValue(value, used);
                return true;
            }
            return false;
        }
    }
}
