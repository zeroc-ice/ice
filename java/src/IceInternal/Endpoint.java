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

public abstract class Endpoint implements java.lang.Comparable
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
    endpointFromString(Instance instance, String str)
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

        String protocol = s.substring(0, m.start());

        if (protocol.equals("default"))
        {
            protocol = instance.defaultProtocol();
        }

        if (protocol.equals("tcp"))
        {
            return new TcpEndpoint(instance, s.substring(m.end()));
        }

        if (protocol.equals("ssl"))
        {
            // TODO: SSL
            //return new SslEndpoint(instance, s.substring(m.end()));
        }

        if (protocol.equals("udp"))
        {
            return new UdpEndpoint(instance, s.substring(m.end()));
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

            /* TODO: SSL
            case SslEndpointType:
            {
                v = new SslEndpoint(s);
                break;
            }
            */

            case UdpEndpointType:
            {
                v = new UdpEndpoint(s);
                break;
            }

            default:
            {
                v = new UnknownEndpoint(type, s);
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
    // Return true if the endpoint type is unknown.
    //
    public abstract boolean unknown();

    //
    // Return a client side transceiver for this endpoint, or null if a
    // transceiver can only be created by a connector.
    //
    public abstract Transceiver clientTransceiver();

    //
    // Return a server side transceiver for this endpoint, or null if a
    // transceiver can only be created by an acceptor. In case a
    // transceiver is created, this operation also returns a new
    // "effective" endpoint, which might differ from this endpoint,
    // for example, if a dynamic port number is assigned.
    //
    public abstract Transceiver serverTransceiver(EndpointHolder endpoint);

    //
    // Return a connector for this endpoint, or null if no connector
    // is available.
    //
    public abstract Connector connector();

    //
    // Return an acceptor for this endpoint, or null if no acceptors
    // is available. In case an acceptor is created, this operation
    // also returns a new "effective" endpoint, which might differ
    // from this endpoint, for example, if a dynamic port number is
    // assigned.
    //
    public abstract Acceptor acceptor(EndpointHolder endpoint);

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
    public abstract int compareTo(java.lang.Object obj); // From java.lang.Comparable
}
