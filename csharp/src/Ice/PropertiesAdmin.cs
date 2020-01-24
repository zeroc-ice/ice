//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace Ice
{
    public interface INativePropertiesAdmin
    {
        void AddUpdateCallback(Action<Dictionary<string, string>> callback);

        void RemoveUpdateCallback(Action<Dictionary<string, string>> callback);
    }

}

namespace IceInternal
{
    internal sealed class PropertiesAdmin : Ice.IPropertiesAdmin, Ice.INativePropertiesAdmin
    {
        internal PropertiesAdmin(Ice.Communicator communicator)
        {
            _communicator = communicator;
            _logger = communicator.Logger;
        }

        public string
        GetProperty(string name, Ice.Current current) => _communicator.GetProperty(name) ?? "";

        public Dictionary<string, string>
        GetPropertiesForPrefix(string name, Ice.Current current) => _communicator.GetProperties(forPrefix: name);

        public void
        SetProperties(Dictionary<string, string> props, Ice.Current current)
        {
            int? traceLevel = _communicator.GetPropertyAsInt("Ice.Trace.Admin.Properties");

            // Update the communicator's properties and remove the properties that did not change from props.
            _communicator.SetProperties(props);

            if (traceLevel > 0 && props.Count > 0)
            {
                var message = new System.Text.StringBuilder("Summary of property changes");

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

                _logger.Trace(TraceCategory, message.ToString());
            }

            lock (this)
            {
                if (_updateCallbacks.Count > 0)
                {
                    // Copy callbacks to allow callbacks to update callbacks
                    foreach (Action<Dictionary<string, string>> callback in new List<Action<Dictionary<string, string>>>(_updateCallbacks))
                    {
                        try
                        {
                            callback(props);
                        }
                        catch (Exception ex)
                        {
                            if (_communicator.GetPropertyAsInt("Ice.Warn.Dispatch") > 1)
                            {
                                _logger.Warning("properties admin update callback raised unexpected exception:\n" + ex);
                            }
                        }
                    }
                }
            }
        }

        public void AddUpdateCallback(Action<Dictionary<string, string>> cb)
        {
            lock (this)
            {
                _updateCallbacks.Add(cb);
            }
        }

        public void RemoveUpdateCallback(Action<Dictionary<string, string>> cb)
        {
            lock (this)
            {
                _updateCallbacks.Remove(cb);
            }
        }

        private readonly Ice.Communicator _communicator;
        private readonly Ice.ILogger _logger;
        private readonly List<Action<Dictionary<string, string>>> _updateCallbacks =
            new List<Action<Dictionary<string, string>>>();

        private const string TraceCategory = "Admin.Properties";
    }
}
