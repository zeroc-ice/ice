// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;

namespace Ice
{

public interface PropertiesAdminUpdateCallback
{
    void updated(Dictionary<string, string> changes);
}

public interface NativePropertiesAdmin
{
    [Obsolete("This method is deprecated. Use addUpdatecallback(System.Action<Dictionary<string, string>> changes) instead.")]
    void addUpdateCallback(PropertiesAdminUpdateCallback callback);

    [Obsolete("This method is deprecated. Use removeUpdatecallback(System.Action<Dictionary<string, string>> changes) instead.")]
    void removeUpdateCallback(PropertiesAdminUpdateCallback callback);

    void addUpdateCallback(System.Action<Dictionary<string, string>> callback);

    void removeUpdateCallback(System.Action<Dictionary<string, string>> callback);
}

}

namespace IceInternal
{
    sealed class PropertiesAdminI : Ice.PropertiesAdminDisp_, Ice.NativePropertiesAdmin
    {
        internal PropertiesAdminI(Instance instance)
        {
            _properties = instance.initializationData().properties;
            _logger = instance.initializationData().logger;
        }

        public override string
        getProperty(string name, Ice.Current current)
        {
            return _properties.getProperty(name);
        }

        public override Dictionary<string, string>
        getPropertiesForPrefix(string name, Ice.Current current)
        {
            return _properties.getPropertiesForPrefix(name);
        }

        public override void
        setProperties(Dictionary<string, string> props, Ice.Current current)
        {
            lock(this)
            {
                Dictionary<string, string> old = _properties.getPropertiesForPrefix("");
                int traceLevel = _properties.getPropertyAsInt("Ice.Trace.Admin.Properties");

                //
                // Compute the difference between the new property set and the existing property set:
                //
                // 1) Any properties in the new set that were not defined in the existing set.
                //
                // 2) Any properties that appear in both sets but with different values.
                //
                // 3) Any properties not present in the new set but present in the existing set.
                //    In other words, the property has been removed.
                //
                Dictionary<string, string> added = new Dictionary<string, string>();
                Dictionary<string, string> changed = new Dictionary<string, string>();
                Dictionary<string, string> removed = new Dictionary<string, string>();
                foreach(KeyValuePair<string, string> e in props)
                {
                    string key = e.Key;
                    string value = e.Value;
                    if(!old.ContainsKey(key))
                    {
                        if(value.Length > 0)
                        {
                            //
                            // This property is new.
                            //
                            added.Add(key, value);
                        }
                    }
                    else
                    {
                        string v;
                        if(!old.TryGetValue(key, out v) || !value.Equals(v))
                        {
                            if(value.Length == 0)
                            {
                                //
                                // This property was removed.
                                //
                                removed.Add(key, value);
                            }
                            else
                            {
                                //
                                // This property has changed.
                                //
                                changed.Add(key, value);
                            }
                        }

                        old.Remove(key);
                    }
                }

                if(traceLevel > 0 && (added.Count > 0 || changed.Count > 0 || removed.Count > 0))
                {
                    System.Text.StringBuilder message = new System.Text.StringBuilder("Summary of property changes");

                    if(added.Count > 0)
                    {
                        message.Append("\nNew properties:");
                        foreach(KeyValuePair<string, string> e in added)
                        {
                            message.Append("\n  ");
                            message.Append(e.Key);
                            if(traceLevel > 1)
                            {
                                message.Append(" = ");
                                message.Append(e.Value);
                            }
                        }
                    }

                    if(changed.Count > 0)
                    {
                        message.Append("\nChanged properties:");
                        foreach(KeyValuePair<string, string> e in changed)
                        {
                            message.Append("\n  ");
                            message.Append(e.Key);
                            if(traceLevel > 1)
                            {
                                message.Append(" = ");
                                message.Append(e.Value);
                                message.Append(" (old value = ");
                                message.Append(_properties.getProperty(e.Key));
                                message.Append(")");
                            }
                        }
                    }

                    if(removed.Count > 0)
                    {
                        message.Append("\nRemoved properties:");
                        foreach(KeyValuePair<string, string> e in removed)
                        {
                            message.Append("\n  ");
                            message.Append(e.Key);
                        }
                    }

                    _logger.trace(_traceCategory, message.ToString());
                }

                //
                // Update the property set.
                //

                foreach(KeyValuePair<string, string> e in added)
                {
                    _properties.setProperty(e.Key, e.Value);
                }

                foreach(KeyValuePair<string, string> e in changed)
                {
                    _properties.setProperty(e.Key, e.Value);
                }

                foreach(KeyValuePair<string, string> e in removed)
                {
                    _properties.setProperty(e.Key, "");
                }

                if(_deprecatedUpdateCallbacks.Count > 0 || _updateCallbacks.Count > 0)
                {
                    Dictionary<string, string> changes = new Dictionary<string, string>(added);
                    foreach(KeyValuePair<string, string> e in changed)
                    {
                        changes.Add(e.Key, e.Value);
                    }
                    foreach(KeyValuePair<string, string> e in removed)
                    {
                        changes.Add(e.Key, e.Value);
                    }

                    // Copy callbacks to allow callbacks to update callbacks
                    foreach(var callback in new List<Ice.PropertiesAdminUpdateCallback>(_deprecatedUpdateCallbacks))
                    {
                        try
                        {
                            callback.updated(changes);
                        }
                        catch(Exception ex)
                        {
                            if(_properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 1)
                            {
                                _logger.warning("properties admin update callback raised unexpected exception:\n" + ex);
                            }
                        }
                    }
                    // Copy callbacks to allow callbacks to update callbacks
                    foreach(var callback in new List<System.Action<Dictionary<string, string>>>(_updateCallbacks))
                    {
                        try
                        {
                            callback(changes);
                        }
                        catch(Exception ex)
                        {
                            if(_properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 1)
                            {
                                _logger.warning("properties admin update callback raised unexpected exception:\n" + ex);
                            }
                        }
                    }
                }
            }
        }

        public void addUpdateCallback(Ice.PropertiesAdminUpdateCallback cb)
        {
            lock(this)
            {
                _deprecatedUpdateCallbacks.Add(cb);
            }
        }

        public void removeUpdateCallback(Ice.PropertiesAdminUpdateCallback cb)
        {
            lock(this)
            {
                _deprecatedUpdateCallbacks.Remove(cb);
            }
        }

        public void addUpdateCallback(System.Action<Dictionary<string, string>> cb)
        {
            lock(this)
            {
                _updateCallbacks.Add(cb);
            }
        }

        public void removeUpdateCallback(System.Action<Dictionary<string, string>> cb)
        {
            lock(this)
            {
                _updateCallbacks.Remove(cb);
            }
        }

        private readonly Ice.Properties _properties;
        private readonly Ice.Logger _logger;
        private List<Ice.PropertiesAdminUpdateCallback> _deprecatedUpdateCallbacks =
            new List<Ice.PropertiesAdminUpdateCallback>();
        private List<System.Action<Dictionary<string, string>>> _updateCallbacks =
            new List<System.Action<Dictionary<string, string>>>();

        private static readonly string _traceCategory = "Admin.Properties";
    }
}
