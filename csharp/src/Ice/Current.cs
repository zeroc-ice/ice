//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Ice
{
    public class Current
    {
        public ObjectAdapter Adapter { get; }
        public Connection? Connection { get; }
        public Identity Id { get; }
        public string Facet { get; }
        public string Operation { get; }
        public bool IsIdempotent { get; }
        public Dictionary<string, string> Context { get; }
        public int RequestId { get; }
        public bool IsOneway => RequestId == 0;
        public EncodingVersion Encoding { get; }

        internal Current(ObjectAdapter adapter, Identity id, string facet, string operation, bool idempotent,
            Dictionary<string, string> ctx, int requestId, Connection? connection, EncodingVersion encoding)
        {
            Adapter = adapter;
            Id = id;
            Facet = facet;
            Operation = operation;
            IsIdempotent = idempotent;
            Context = ctx;
            RequestId = requestId;
            Connection = connection;
            Encoding = encoding;
        }
    }
}
