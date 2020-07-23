//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading;
using System.Collections.Generic;

namespace ZeroC.Ice
{
    public class Current
    {
        public ObjectAdapter Adapter { get; }
        public CancellationToken CancellationToken { get; }
        public Communicator Communicator => Adapter.Communicator;
        public Connection? Connection { get; }
        // TODO: should this be a IReadOnlyDictionary<string, string>?
        public Dictionary<string, string> Context { get; }
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
            bool oneway,
            CancellationToken cancel,
            Connection? connection = null)
        {
            Adapter = adapter;
            CancellationToken = cancel;
            Connection = connection;
            Context = new Dictionary<string, string>(incomingRequestFrame.Context);
            IsOneway = oneway;
            IncomingRequestFrame = incomingRequestFrame;
        }
    }
}
