// Copyright (c) ZeroC, Inc. All rights reserved.
using System;

namespace ZeroC.Ice
{
    public partial class ObjectNotExistException
    {
        /// <inheritdoc/>
        protected override string DefaultMessage =>
            $"could not find servant for Ice object `{Origin!.Value.Identity}'" +
            (Origin.Value.Facet.Length > 0 ? $" with facet `{Origin.Value.Facet}'" : "") +
            $" while attempting to dispatch operation `{Origin.Value.Operation}'";
    }

    public partial class OperationNotExistException
    {
        /// <inheritdoc/>
        protected override string DefaultMessage =>
            $"could not find operation `{Origin!.Value.Operation}' for Ice object `{Origin.Value.Identity}'" +
            (Origin.Value.Facet.Length > 0 ? $" with facet `{Origin.Value.Facet}'" : "");
    }

    public partial class UnhandledException
    {
        /// <summary>Constructs a new exception.</summary>
        /// <param name="identity">The Ice object Identity.</param>
        /// <param name="facet">The Ice object facet.</param>
        /// <param name="operation">The operation name.</param>
        /// <param name="innerException">The exception that is the cause of the current exception.</param>
        public UnhandledException(Identity identity, string facet, string operation, Exception innerException)
            : base(CustomMessage(identity, facet, operation, innerException), innerException)
        {
        }

        private static string CustomMessage(Identity identity, string facet, string operation, Exception innerException)
        {
            string message = $"unhandled exception while dispatching `{operation}' on Ice object `{identity}'";
            if (facet.Length > 0)
            {
                message += $" with facet `{facet}'";
            }
#if DEBUG
            message += $":\n{innerException}\n---";
#else
            // The stack trace of the inner exception can include sensitive information we don't want to send
            // "over the wire" in non-debug builds.
            message += $":\n{innerException.Message}";
#endif
            return message;
        }
    }
}
