//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Threading;

namespace ZeroC.Ice
{
    public class Current
    {
        public ObjectAdapter Adapter { get; }
        public CancellationToken CancellationToken { get; }
        public Communicator Communicator => Adapter.Communicator;
        public Connection? Connection { get; }
        public Dictionary<string, string> Context => IncomingRequestFrame.Context;
        public Encoding Encoding => IncomingRequestFrame.Encoding;
        public string Facet => IncomingRequestFrame.Facet;
        public Identity Identity => IncomingRequestFrame.Identity;
        public bool IsIdempotent => IncomingRequestFrame.IsIdempotent;
        public bool IsOneway { get; }
        public string Operation => IncomingRequestFrame.Operation;
        public Protocol Protocol => IncomingRequestFrame.Protocol;

        internal IncomingRequestFrame IncomingRequestFrame { get; }

        internal Current(
            ObjectAdapter adapter,
            IncomingRequestFrame incomingRequestFrame,
            Stream stream,
            Connection? connection = null)
        {
            Adapter = adapter;
            CancellationToken = stream.CancellationToken;
            Connection = connection;
            IsOneway = !stream.IsBidirectional;
            IncomingRequestFrame = incomingRequestFrame;
        }
    }
}
