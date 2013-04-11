// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

abstract public class EndpointI implements Ice.Endpoint, java.lang.Comparable<EndpointI>
{
    public EndpointI(String connectionId)
    {
        _connectionId = connectionId;
    }

    public EndpointI()
    {
    }

    public String
    toString()
    {
        return _toString();
    }

    //
    // Marshal the endpoint.
    //
    public abstract void streamWrite(BasicStream s);

    //
    // Return the endpoint type.
    //
    public abstract short type();
    
    //
    // Return the protocol name.
    //
    public abstract String protocol();
    
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
    public abstract EndpointI connectionId(String connectionId);

    //
    // Return true if the endpoints support bzip2 compress, or false
    // otherwise.
    //
    public abstract boolean compress();
    
    //
    // Return a new endpoint with a different compression value,
    // provided that compression is supported by the
    // endpoint. Otherwise the same endpoint is returned.
    //
    public abstract EndpointI compress(boolean co);

    //
    // Return true if the endpoint is datagram-based.
    //
    public abstract boolean datagram();

    //
    // Return true if the endpoint is secure.
    //
    public abstract boolean secure();

    //
    // Return the connection ID 
    //
    public String connectionId()
    {
        return _connectionId;
    }

    //
    // Return a server side transceiver for this endpoint, or null if a
    // transceiver can only be created by an acceptor. In case a
    // transceiver is created, this operation also returns a new
    // "effective" endpoint, which might differ from this endpoint,
    // for example, if a dynamic port number is assigned.
    //
    public abstract Transceiver transceiver(EndpointIHolder endpoint);

    //
    // Return connectors for this endpoint, or empty list if no connector
    // is available.
    //
    public abstract java.util.List<Connector> connectors(Ice.EndpointSelectionType selType);
    public abstract void connectors_async(Ice.EndpointSelectionType selType, EndpointI_connectors callback);

    //
    // Return an acceptor for this endpoint, or null if no acceptors
    // is available. In case an acceptor is created, this operation
    // also returns a new "effective" endpoint, which might differ
    // from this endpoint, for example, if a dynamic port number is
    // assigned.
    //
    public abstract Acceptor acceptor(EndpointIHolder endpoint, String adapterName);

    //
    // Expand endpoint out in to separate endpoints for each local
    // host if listening on INADDR_ANY.
    //
    public abstract java.util.List<EndpointI> expand();

    //
    // Check whether the endpoint is equivalent to another one.
    //
    public abstract boolean equivalent(EndpointI endpoint);

    public java.util.List<Connector>
    connectors(java.util.List<java.net.InetSocketAddress> addresses, NetworkProxy proxy)
    {
        //
        // This method must be extended by endpoints which use the EndpointHostResolver to create
        // connectors from IP addresses.
        //
        assert(false);
        return null;
    }

    //
    // Compare endpoints for sorting purposes.
    //
    public boolean equals(java.lang.Object obj)
    {
        if(!(obj instanceof EndpointI))
        {
            return false;
        }
        return compareTo((EndpointI)obj) == 0;
    }

    public int compareTo(EndpointI p) // From java.lang.Comparable. 
    {
        if(!_connectionId.equals(p._connectionId))
        {
            return _connectionId.compareTo(p._connectionId);
        }

        return 0;
    }

    protected String _connectionId = "";
}
