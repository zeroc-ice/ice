//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    using System.Collections.Generic;

    public class Current
    {
        public ObjectAdapter adapter;
        public Connection con;
        public Identity id;
        public string facet;
        public string operation;
        public OperationMode mode;
        public Dictionary<string, string> ctx;
        public int requestId;
        public EncodingVersion encoding;

        public Current()
        {
            id = new Identity();
            facet = "";
            operation = "";
            encoding = new EncodingVersion();
        }

        public Current(ObjectAdapter adapter, Connection con, Identity id, string facet, string operation,
                       OperationMode mode, Dictionary<string, string> ctx, int requestId, EncodingVersion encoding)
        {
            this.adapter = adapter;
            this.con = con;
            this.id = id;
            this.facet = facet;
            this.operation = operation;
            this.mode = mode;
            this.ctx = ctx;
            this.requestId = requestId;
            this.encoding = encoding;
        }
    }
}
