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

public final class UdpEndpoint extends Endpoint
{
    public
    UdpEndpoint(String ho, int po)
    {
        _host = ho;
        _port = po;
    }

    public
    UdpEndpoint(String str)
    {
        _host = null;
        _port = 0;

        String[] arr = str.split("[ \t\n\r]+");

        int i = 0;
        while (i < arr.length)
        {
            String option = arr[i++];
            if (option.length() != 2 || option.charAt(0) != '-')
            {
                throw new Ice.EndpointParseException();
            }

            String argument = null;
            if (i < arr.length && arr[i].charAt(0) != '-')
            {
                argument = arr[i++];
            }

            switch (option.charAt(1))
            {
                case 'h':
                {
                    if (argument == null)
                    {
                        throw new Ice.EndpointParseException();
                    }

                    _host = argument;
                    break;
                }

                case 'p':
                {
                    if (argument == null)
                    {
                        throw new Ice.EndpointParseException();
                    }

                    try
                    {
                        _port = Integer.parseInt(argument);
                    }
                    catch (NumberFormatException ex)
                    {
                        throw new Ice.EndpointParseException();
                    }

                    break;
                }

                default:
                {
                    throw new Ice.EndpointParseException();
                }
            }
        }

        if (_host == null)
        {
            // TODO: Whether numeric or not should be configurable
            _host = Network.getLocalHost(true);
        }
    }

    public
    UdpEndpoint(BasicStream s)
    {
        s.startReadEncaps();
        _host = s.readString();
        _port = s.readInt();
        s.endReadEncaps();
    }

    //
    // Marshal the endpoint
    //
    public void
    streamWrite(BasicStream s)
    {
        s.writeShort(UdpEndpointType);
        s.startWriteEncaps();
        s.writeString(_host);
        s.writeInt(_port);
        s.endWriteEncaps();
    }

    //
    // Convert the endpoint to its string form
    //
    public String
    toString()
    {
        StringBuffer s = new StringBuffer();
        s.append("udp");
        // TODO: Whether numeric or not should be configurable
        if (!_host.equals(Network.getLocalHost(true)))
        {
            s.append(" -h " + _host);
        }
        if (_port != 0)
        {
            s.append(" -p " + _port);
        }
        return s.toString();
    }

    //
    // Return the endpoint type
    //
    public short
    type()
    {
        return UdpEndpointType;
    }

    //
    // Return true if the endpoint only supports oneway operations.
    //
    public boolean
    oneway()
    {
        return true;
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
    timeout(int timeout)
    {
        return this;
    }

    //
    // Return true if the endpoint is datagram-based.
    //
    public boolean
    datagram()
    {
        return true;
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
        //return new UdpTransceiver(instance, _host, _port);
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
        /*
        UdpTransceiver p = new UdpTransceiver(instance, _port);
        endpoint.value = new UdpEndpoint(_host, p.effectivePort());
        return p;
        */
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
        /*
        UdpTransceiver udpTransceiver = null;
        try
        {
            udpTransceiver = (UdpTransceiver)transceiver;
        }
        catch (ClassCastException ex)
        {
            return false;
        }
        return udpTransceiver.equivalent(_host, _port);
        */
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
        UdpEndpoint p = null;

        try
        {
            p = (UdpEndpoint)obj;
        }
        catch (ClassCastException ex)
        {
            return false;
        }

        if (this == p)
        {
            return true;
        }

        if (_port != p._port)
        {
            return false;
        }

        if (!_host.equals(p._host))
        {
            try
            {
                java.net.InetAddress addr1 =
                    java.net.InetAddress.getByName(_host);

                java.net.InetAddress addr2 =
                    java.net.InetAddress.getByName(p._host);

                if(!addr1.equals(addr2))
                    return false;
            }
            catch(java.net.UnknownHostException ex)
            {
                return false;
            }
        }

        return true;
    }

    private String _host;
    private int _port;
}
