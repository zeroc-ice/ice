//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Context = System.Collections.Generic.Dictionary<string, string>;

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
        public Context Context { get; } // TODO: should this be a ReadOnlyContext?
        public int RequestId { get; }
        public bool IsOneway => RequestId == 0;
        public Encoding Encoding { get; }

        internal Current(int requestId, InputStream request, ObjectAdapter adapter, Ice.Connection? connection = null)
        {
            Adapter = adapter;
            Id = new Identity(request);

            // For compatibility with the old FacetPath.
            string[] facetPath = request.ReadStringArray();
            if (facetPath.Length > 1)
            {
                throw new MarshalException();
            }
            Facet = facetPath.Length == 0 ? "" : facetPath[0];
            Operation = request.ReadString();
            IsIdempotent = request.ReadOperationMode() != OperationMode.Normal;
            Context = request.ReadContext();
            RequestId = requestId;
            Connection = connection;
            Encoding = request.StartEncapsulation();
        }
    }
}
