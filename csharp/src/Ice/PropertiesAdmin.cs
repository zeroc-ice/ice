//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace Ice
{
    /// <summary>Implementations of the Properties admin facet implement both IPropertiesAdmin (which corresponds
    /// to the Slice interface PropertiesAdmin) and the local INativePropertiesAdmin interface.</summary>
    public interface INativePropertiesAdmin
    {
        /// <summary>The Updated event is triggered when the communicator's properties are updated through a remote call
        /// to the setProperties operation implemented by this Properties admin facet. The event's args include only the
        /// communicator properties that were updated by this remote call.</summary>
        public abstract event EventHandler<IReadOnlyDictionary<string, string>>? Updated;
    }

    // Default implementation of the Properties Admin facet.
    internal sealed class PropertiesAdmin : IPropertiesAdmin, INativePropertiesAdmin
    {
        public event EventHandler<IReadOnlyDictionary<string, string>>? Updated;

        private const string TraceCategory = "Ice.Admin.Properties";
        private readonly Communicator _communicator;
        private readonly ILogger _logger;

        public string GetProperty(string name, Current current) => _communicator.GetProperty(name) ?? "";

        public Dictionary<string, string> GetPropertiesForPrefix(string name, Current current) =>
            _communicator.GetProperties(forPrefix: name);

        public void SetProperties(Dictionary<string, string> props, Current current)
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

            Updated?.Invoke(_communicator, props);
        }

        internal PropertiesAdmin(Communicator communicator)
        {
            _communicator = communicator;
            _logger = communicator.Logger;
        }
    }
}
