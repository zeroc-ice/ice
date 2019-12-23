//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace Ice
{
    public interface NativePropertiesAdmin
    {
        void addUpdateCallback(System.Action<Dictionary<string, string>> callback);

        void removeUpdateCallback(System.Action<Dictionary<string, string>> callback);
    }

}

namespace IceInternal
{
    internal sealed class PropertiesAdminI : Ice.PropertiesAdmin, Ice.NativePropertiesAdmin
    {
        internal PropertiesAdminI(Ice.Communicator communicator)
        {
            _communicator = communicator;
            _logger = communicator.Logger;
        }

        public string
        getProperty(string name, Ice.Current current)
        {
            return _communicator.GetProperty(name) ?? "";
        }

        public Dictionary<string, string>
        getPropertiesForPrefix(string name, Ice.Current current)
        {
            return _communicator.GetProperties(forPrefix: name);
        }

        public void
        setProperties(Dictionary<string, string> props, Ice.Current current)
        {
            int? traceLevel = _communicator.GetPropertyAsInt("Ice.Trace.Admin.Properties");

            // Update the communicator's properties and remove the properties that did not change from props.
            _communicator.SetProperties(props);

            if (traceLevel > 0 && props.Count > 0)
            {
                System.Text.StringBuilder message = new System.Text.StringBuilder("Summary of property changes");

                message.Append("\nNew or updated properties:");
                foreach (KeyValuePair<string, string> e in props)
                {
                    if (e.Value.Length > 0)
                    {
                        message.Append("\n  ");
                        message.Append(e.Key);
                        if (traceLevel > 1)
                        {
                            message.Append(" = ");
                            message.Append(e.Value);
                        }
                    }
                }

                message.Append("\nRemoved properties:");
                foreach (KeyValuePair<string, string> e in props)
                {
                    if (e.Value.Length == 0)
                    {
                        message.Append("\n  ");
                        message.Append(e.Key);
                    }
                }

                _logger.trace(_traceCategory, message.ToString());
            }

            lock (this)
            {
                if (_updateCallbacks.Count > 0)
                {
                    // Copy callbacks to allow callbacks to update callbacks
                    foreach (var callback in new List<Action<Dictionary<string, string>>>(_updateCallbacks))
                    {
                        try
                        {
                            callback(props);
                        }
                        catch (Exception ex)
                        {
                            if (_communicator.GetPropertyAsInt("Ice.Warn.Dispatch") > 1)
                            {
                                _logger.warning("properties admin update callback raised unexpected exception:\n" + ex);
                            }
                        }
                    }
                }
            }
        }

        public void addUpdateCallback(System.Action<Dictionary<string, string>> cb)
        {
            lock (this)
            {
                _updateCallbacks.Add(cb);
            }
        }

        public void removeUpdateCallback(System.Action<Dictionary<string, string>> cb)
        {
            lock (this)
            {
                _updateCallbacks.Remove(cb);
            }
        }

        private readonly Ice.Communicator _communicator;
        private readonly Ice.Logger _logger;
        private List<Action<Dictionary<string, string>>> _updateCallbacks =
            new List<Action<Dictionary<string, string>>>();

        private const string _traceCategory = "Admin.Properties";
    }
}
