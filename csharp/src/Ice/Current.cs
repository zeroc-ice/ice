//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    using System.Collections.Generic;

    public class Current
    {
        public ObjectAdapter Adapter { get; private set; }
        public Connection? Connection { get; private set; }
        public Identity Id { get; private set; }
        public string Facet { get; private set; }
        public string Operation { get; private set; }
        public OperationMode Mode { get; private set; }
        public Dictionary<string, string> Context { get; private set; }
        public int RequestId { get; private set; }
        public EncodingVersion Encoding { get; internal set; }

        public Current(ObjectAdapter adapter, Identity id, string facet, string operation,
            OperationMode mode, Dictionary<string, string> ctx, int requestId, Connection? conn)
        {
            Adapter = adapter;
            Id = id;
            Facet = facet;
            Operation = operation;
            Mode = mode;
            Context = ctx;
            RequestId = requestId;
            Connection = conn;
        }
    }
}
