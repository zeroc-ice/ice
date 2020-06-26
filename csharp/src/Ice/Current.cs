//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace ZeroC.Ice
{
    public class Current
    {
        public ObjectAdapter Adapter { get; }
        public Connection? Connection { get; }
        // TODO: should this be a IReadOnlyDictionary<string, string>?
        public Dictionary<string, string> Context { get; }
        public Encoding Encoding { get; }
        public string Facet { get; }
        public Identity Identity { get; }
        public bool IsIdempotent { get; }
        public bool IsOneway => RequestId == 0;
        public string Operation { get; }
        public Protocol Protocol { get; }

        public int RequestId { get; }

        internal Current(ObjectAdapter adapter, IncomingRequestFrame request, int requestId,
            Connection? connection = null)
        {
            Adapter = adapter;
            Connection = connection;
            Context = new Dictionary<string, string>(request.Context);
            Encoding = request.Encoding;
            Facet = request.Facet;
            Identity = request.Identity;
            IsIdempotent = request.IsIdempotent;
            Operation = request.Operation;
            Protocol = request.Protocol;
            RequestId = requestId;
        }
    }
}
