// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading;

namespace ZeroC.Ice
{
    /// <summary>Information about the current method dispatch for servers. Each method on the server has a
    /// Current as its final parameter.</summary>
    public class Current
    {
        /// <summary>The object adapter.</summary>
        public ObjectAdapter Adapter { get; }

        /// <summary>The communicator.</summary>
        public Communicator Communicator => Adapter.Communicator;

        /// <summary>The <see cref="Connection"/> over which the request was dispatched; it is null for colocated
        /// dispatches.</summary>
        public Connection? Connection { get; }

        /// <summary>The request deadline. The peer sets the deadline to the absolute time at which its timeout will be
        /// triggered. With Ice1 the peer doesn't send a deadline and it always has the default value.</summary>
        public DateTime Deadline => IncomingRequestFrame.Deadline;

        /// <summary>The request context, as received from the client.</summary>
        public Dictionary<string, string> Context => IncomingRequestFrame.Context;

        /// <summary>The encoding used by the request.</summary>
        public Encoding Encoding => IncomingRequestFrame.Encoding;

        /// <summary>The Ice object facet.</summary>
        public string Facet => IncomingRequestFrame.Facet;

        /// <summary>The Ice object Identity.</summary>
        public Identity Identity => IncomingRequestFrame.Identity;

        /// <summary><c>True</c> if the operation was marked as idempotent, <c>False</c> otherwise.</summary>
        public bool IsIdempotent => IncomingRequestFrame.IsIdempotent;

        /// <summary><c>True</c> for oneway requests, <c>False</c> otherwise.</summary>
        public bool IsOneway { get; }

        /// <summary>The location of the target Ice object.</summary>
        public string[] Location => IncomingRequestFrame.Location;

        /// <summary>The operation name.</summary>
        public string Operation => IncomingRequestFrame.Operation;

        /// <summary>The protocol used by the request.</summary>
        public Protocol Protocol => IncomingRequestFrame.Protocol;

        internal IncomingRequestFrame IncomingRequestFrame { get; }

        internal Current(
            ObjectAdapter adapter,
            IncomingRequestFrame incomingRequestFrame,
            bool oneway,
            Connection? connection = null)
        {
            Adapter = adapter;
            Connection = connection;
            IsOneway = oneway;
            IncomingRequestFrame = incomingRequestFrame;
        }
    }
}
