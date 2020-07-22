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
        public Encoding Encoding => Request.Encoding;
        public string Facet => Request.Facet;
        public Identity Identity => Request.Identity;
        public bool IsIdempotent => Request.IsIdempotent;
        public bool IsOneway { get; }
        public string Operation => Request.Operation;
        public Protocol Protocol => Request.Protocol;

        internal IncomingRequestFrame Request { get; }

        internal Current(
            ObjectAdapter adapter,
            IncomingRequestFrame request,
            bool oneway,
            CancellationToken cancel,
            Connection? connection = null)
        {
            Adapter = adapter;
            CancellationToken = cancel;
            Connection = connection;
            Context = new Dictionary<string, string>(request.Context);
            IsOneway = oneway;
            Request = request;
        }
    }
}
