// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    // This partial interface extends the IAsyncPropertieAdmin interface generated from the Slice interface
    // PropertiesAdmin.
    public partial interface IAsyncPropertiesAdmin
    {
        /// <summary>The Updated event is triggered when the communicator's properties are updated through a remote call
        /// to the setProperties operation implemented by this Properties admin facet. The event's args include only the
        /// communicator properties that were updated by this remote call.</summary>
        public abstract event EventHandler<IReadOnlyDictionary<string, string>>? Updated;
    }

    // Default implementation of the Properties Admin facet.
    internal sealed class PropertiesAdmin : IAsyncPropertiesAdmin
    {
        public event EventHandler<IReadOnlyDictionary<string, string>>? Updated;

        private const string TraceCategory = "Ice.Admin.Properties";
        private readonly Communicator _communicator;
        private readonly ILogger _logger;

        public ValueTask<string> GetPropertyAsync(string key, Current current, CancellationToken cancel) =>
            new(_communicator.GetProperty(key) ?? "");

        public ValueTask<IReadOnlyDictionary<string, string>> GetPropertiesForPrefixAsync(
            string prefix,
            Current current,
            CancellationToken cancel) =>
            new(_communicator.GetProperties(forPrefix: prefix));

        public ValueTask SetPropertiesAsync(
            Dictionary<string, string> newProperties,
            Current current,
            CancellationToken cancel)
        {
            int? traceLevel = _communicator.GetPropertyAsInt("Ice.Trace.Admin.Properties");

            // Update the communicator's properties and remove the properties that did not change from props.
            _communicator.SetProperties(newProperties);

            if (traceLevel > 0 && newProperties.Count > 0)
            {
                var message = new System.Text.StringBuilder("Summary of property changes");

                message.Append("\nNew or updated properties:");
                foreach (KeyValuePair<string, string> e in newProperties)
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
                foreach (KeyValuePair<string, string> e in newProperties)
                {
                    if (e.Value.Length == 0)
                    {
                        message.Append("\n  ");
                        message.Append(e.Key);
                    }
                }

                _logger.Trace(TraceCategory, message.ToString());
            }

            Updated?.Invoke(_communicator, newProperties);
            return default;
        }

        internal PropertiesAdmin(Communicator communicator)
        {
            _communicator = communicator;
            _logger = communicator.Logger;
        }
    }
}
