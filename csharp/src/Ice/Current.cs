// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;

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

        /// <summary>The <see cref="Connection"/> over which the request was dispatched.</summary>
        public Connection Connection { get; }

        /// <summary>The request context, as received from the client.</summary>
        public SortedDictionary<string, string> Context => IncomingRequestFrame.Context;

        /// <summary>The deadline corresponds to the request's expiration time. Once the deadline is reached, the
        /// caller is no longer interested in the response and discards the request. The server-side runtime does not
        /// enforce this deadline - it's provided "for information" to the application. The Ice client runtime sets
        /// this deadline automatically using the proxy's invocation timeout and sends it with ice2 requests but not
        /// with ice1 requests. As a result, the deadline for an ice1 request is always <see cref="DateTime.MaxValue"/>
        /// on the server-side even though the invocation timeout is usually not infinite.</summary>
        public DateTime Deadline => IncomingRequestFrame.Deadline;

        /// <summary>The encoding used by the request.</summary>
        public Encoding Encoding => IncomingRequestFrame.PayloadEncoding;

        /// <summary>The Ice object facet.</summary>
        public string Facet => IncomingRequestFrame.Facet;

        /// <summary>The Ice object Identity.</summary>
        public Identity Identity => IncomingRequestFrame.Identity;

        /// <summary><c>True</c> if the operation was marked as idempotent, <c>False</c> otherwise.</summary>
        public bool IsIdempotent => IncomingRequestFrame.IsIdempotent;

        /// <summary><c>True</c> for oneway requests, <c>False</c> otherwise.</summary>
        public bool IsOneway => !Stream.IsBidirectional;

        /// <summary>The location of the target Ice object.</summary>
        public IReadOnlyList<string> Location => IncomingRequestFrame.Location;

        /// <summary>The operation name.</summary>
        public string Operation => IncomingRequestFrame.Operation;

        /// <summary>The protocol used by the request.</summary>
        public Protocol Protocol => IncomingRequestFrame.Protocol;
        /// <summary>The stream ID</summary>
        public long StreamId => Stream.Id;
        internal IncomingRequestFrame IncomingRequestFrame { get; }
        internal SocketStream Stream { get; }

        internal Current(
            ObjectAdapter adapter,
            IncomingRequestFrame incomingRequestFrame,
            SocketStream stream,
            Connection connection)
        {
            Adapter = adapter;
            Connection = connection;
            IncomingRequestFrame = incomingRequestFrame;
            Stream = stream;
        }
    }
}
