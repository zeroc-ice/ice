// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

public final class UnknownEndpoint extends Endpoint
{
    public
    UnknownEndpoint(Ice.Stream s)
    {
        _rawBytes = s.readByteSeq();
    }

    //
    // Marshal the endpoint
    //
    public void
    streamWrite(Ice.Stream s)
    {
        s.writeShort(UnknownEndpointType);
        s.writeByteSeq(_rawBytes);
    }

    //
    // Convert the endpoint to its string form
    //
    public String
    toString()
    {
        return "";
    }

    //
    // Return the endpoint type
    //
    public short
    type()
    {
        return UnknownEndpointType;
    }

    //
    // Return true if the endpoint only supports oneway operations.
    //
    public boolean
    oneway()
    {
        return false;
    }

    //
    // Return the timeout for the endpoint in milliseconds. 0 means
    // non-blocking, -1 means no timeout.
    //
    public int
    timeout()
    {
        return -1;
    }
    
    //
    // Return a new endpoint with a different timeout value, provided
    // that timeouts are supported by the endpoint. Otherwise the same
    // endpoint is returned.
    //
    public Endpoint
    timeout(int t)
    {
        return this;
    }
    
    //
    // Return true if the endpoint is datagram-based.
    //
    public boolean
    datagram()
    {
        return false;
    }
    
    //
    // Return true if the endpoint is secure.
    //
    public boolean
    secure()
    {
        return false;
    }
    
    //
    // Return a client side transceiver for this endpoint, or null if a
    // transceiver can only be created by a connector.
    //
    public Transceiver
    clientTransceiver(Instance instance)
    {
        return null;
    }

    //
    // Return a server side transceiver for this endpoint, or null if a
    // transceiver can only be created by an acceptor. In case a
    // transceiver is created, this operation also returns a new
    // "effective" endpoint, which might differ from this endpoint,
    // for example, if a dynamic port number is assigned.
    //
    public Transceiver
    serverTransceiver(Instance instance, EndpointHolder endpoint)
    {
        endpoint.value = null;
        return null;
    }

    //
    // Return a connector for this endpoint, or null if no connector
    // is available.
    //
    public Connector
    connector(Instance instance)
    {
        return null;
    }

    //
    // Return an acceptor for this endpoint, or null if no acceptors
    // is available. In case an acceptor is created, this operation
    // also returns a new "effective" endpoint, which might differ
    // from this endpoint, for example, if a dynamic port number is
    // assigned.
    //
    public Acceptor
    acceptor(Instance instance, EndpointHolder endpoint)
    {
        endpoint.value = null;
        return null;
    }

    //
    // Check whether the endpoint is equivalent to a specific
    // Transceiver or Acceptor
    //
    public boolean
    equivalent(Transceiver transceiver)
    {
        return false;
    }

    public boolean
    equivalent(Acceptor acceptor)
    {
        return false;
    }
    
    //
    // Compare endpoints for sorting purposes
    //
    public boolean
    equals(java.lang.Object obj)
    {
        UnknownEndpoint p = null;

        try
        {
            p = (UnknownEndpoint)obj;
        }
        catch (ClassCastException ex)
        {
            return false;
        }

        if (this == p)
        {
            return true;
        }

        if (!java.util.Arrays.equals(_rawBytes, p._rawBytes))
        {
            return false;
        }

        return true;
    }

    private byte[] _rawBytes;
}
