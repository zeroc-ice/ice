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

public abstract class Endpoint
{
    public static final short UnknownEndpointType = 0;
    public static final short TcpEndpointType = 1;
    public static final short SslEndpointType = 2;
    public static final short UdpEndpointType = 3;

    public
    Endpoint()
    {
    }

    //
    // Create an endpoint from a string
    //
    public static Endpoint
    endpointFromString(String str)
    {
        String s = str.trim();
        if (s.length() == 0)
        {
            throw new Ice.EndpointParseException();
        }

        java.util.regex.Pattern p =
            java.util.regex.Pattern.compile("([ \t\n\r]+)|$");
        java.util.regex.Matcher m = p.matcher(s);
        boolean b = m.find();
        assert(b);

        String type = s.substring(0, m.start());
        if (type.equals("tcp"))
        {
            return new TcpEndpoint(s.substring(m.end()));
        }

        if (type.equals("ssl"))
        {
            // TODO: SSL
            //return new SslEndpoint(s.substring(m.end()));
        }

        if (type.equals("udp"))
        {
            return new UdpEndpoint(s.substring(m.end()));
        }

        throw new Ice.EndpointParseException();
    }

    //
    // Unmarshal an endpoint
    //
    public static Endpoint
    streamRead(BasicStream s)
    {
        Endpoint v;
        short type = s.readShort();

        switch (type)
        {
            case TcpEndpointType:
            {
                v = new TcpEndpoint(s);
                break;
            }

            case SslEndpointType:
            {
                // TODO: SSL
                //v = new SslEndpoint(s);
                v = null;
                break;
            }

            case UdpEndpointType:
            {
                v = new UdpEndpoint(s);
                break;
            }

            default:
            {
                v = new UnknownEndpoint(s);
                break;
            }
        }

        return v;
    }

    //
    // Marshal the endpoint
    //
    public abstract void streamWrite(BasicStream s);

    //
    // Convert the endpoint to its string form
    //
    public abstract String toString();

    //
    // Return the endpoint type
    //
    public abstract short type();

    //
    // Return true if the endpoint only supports oneway operations.
    //
    public abstract boolean oneway();

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
    public abstract Endpoint timeout(int t);
    
    //
    // Return true if the endpoint is datagram-based.
    //
    public abstract boolean datagram();
    
    //
    // Return true if the endpoint is secure.
    //
    public abstract boolean secure();
    
    //
    // Return a client side transceiver for this endpoint, or null if a
    // transceiver can only be created by a connector.
    //
    public abstract Transceiver clientTransceiver(Instance instance);

    //
    // Return a server side transceiver for this endpoint, or null if a
    // transceiver can only be created by an acceptor. In case a
    // transceiver is created, this operation also returns a new
    // "effective" endpoint, which might differ from this endpoint,
    // for example, if a dynamic port number is assigned.
    //
    public abstract Transceiver serverTransceiver(Instance instance,
                                                  EndpointHolder endpoint);

    //
    // Return a connector for this endpoint, or null if no connector
    // is available.
    //
    public abstract Connector connector(Instance instance);

    //
    // Return an acceptor for this endpoint, or null if no acceptors
    // is available. In case an acceptor is created, this operation
    // also returns a new "effective" endpoint, which might differ
    // from this endpoint, for example, if a dynamic port number is
    // assigned.
    //
    public abstract Acceptor acceptor(Instance instance,
                                      EndpointHolder endpoint);

    //
    // Check whether the endpoint is equivalent to a specific
    // Transceiver or Acceptor
    //
    public abstract boolean equivalent(Transceiver transceiver);
    public abstract boolean equivalent(Acceptor acceptor);
    
    //
    // Compare endpoints for sorting purposes
    //
    public abstract boolean equals(java.lang.Object obj);
}
