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
        public Encoding Encoding { get; }
        public string Facet { get; }
        public Identity Identity { get; }
        public bool IsIdempotent { get; }
        public bool IsOneway { get; }
        public string Operation { get; }
        public Protocol Protocol { get; }

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
            Encoding = request.Encoding;
            Facet = request.Facet;
            Identity = request.Identity;
            IsIdempotent = request.IsIdempotent;
            IsOneway = oneway;
            Operation = request.Operation;
            Protocol = request.Protocol;
        }
    }
}
