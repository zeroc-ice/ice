// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Net;

    public interface EndpointI_connectors
    {
        void connectors(List<Connector> connectors);
        void exception(Ice.LocalException ex);
    }

    public abstract class EndpointI : Ice.Endpoint, System.IComparable
    {    
        public override string ToString()
        {
            return ice_toString_();
        }

        public abstract string ice_toString_();
        public abstract Ice.EndpointInfo getInfo();
        public abstract int CompareTo(object obj);

        //
        // Marshal the endpoint.
        //
        public abstract void streamWrite(BasicStream s);

        //
        // Return the endpoint type.
        //
        public abstract short type();

        //
        // Return the timeout for the endpoint in milliseconds. 0 means
        // non-blocking, -1 means no timeout.
        //
        public abstract int timeout();

        //
        // Return a new endpoint with a different timeout value, provided
        // that timeouts are supported by the endpoint. Otherwise the same
        // endpoint is returned.
        //
        public abstract EndpointI timeout(int t);

        //
        // Return a new endpoint with a different connection id.
        //
        public abstract EndpointI connectionId(string connectionId);

        //
        // Return true if the endpoints support bzip2 compress, or false
        // otherwise.
        //
        public abstract bool compress();

        //
        // Return a new endpoint with a different compression value,
        // provided that compression is supported by the
        // endpoint. Otherwise the same endpoint is returned.
        //
        public abstract EndpointI compress(bool co);

        //
        // Return true if the endpoint is datagram-based.
        //
        public abstract bool datagram();
        
        //
        // Return true if the endpoint is secure.
        //
        public abstract bool secure();

        //
        // Return a server side transceiver for this endpoint, or null if a
        // transceiver can only be created by an acceptor. In case a
        // transceiver is created, this operation also returns a new
        // "effective" endpoint, which might differ from this endpoint,
        // for example, if a dynamic port number is assigned.
        //
        public abstract Transceiver transceiver(ref EndpointI endpoint);

        //
        // Return a connector for this endpoint, or empty list if no connector
        // is available.
        //
        public abstract List<Connector> connectors();
        public abstract void connectors_async(EndpointI_connectors callback);

        //
        // Return an acceptor for this endpoint, or null if no acceptors
        // is available. In case an acceptor is created, this operation
        // also returns a new "effective" endpoint, which might differ
        // from this endpoint, for example, if a dynamic port number is
        // assigned.
        //
        public abstract Acceptor acceptor(ref EndpointI endpoint, string adapterName);

        //
        // Expand endpoint out in to separate endpoints for each local
        // host if listening on INADDR_ANY on server side or if no host
        // was specified on client side.
        //
        public abstract List<EndpointI> expand();
 
        //
        // Check whether the endpoint is equivalent to another one.
        //
        public abstract bool equivalent(EndpointI endpoint);

        public virtual List<Connector> connectors(List<EndPoint> addresses)
        {
            Debug.Assert(false);
            return null;
        }
    }

}
