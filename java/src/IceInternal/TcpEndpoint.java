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

public final class TcpEndpoint extends Endpoint
{
    public
    TcpEndpoint(String ho, int po, int ti)
    {
        _host = ho;
        _port = po;
        _timeout = ti;
    }

    public
    TcpEndpoint(String str)
    {
        _host = null;
        _port = 0;
        _timeout = -1;

        String[] arr = init.split("[ \t\n\r]+");

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

                    host = argument;
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

                case 't':
                {
                    if (argument == null)
                    {
                        throw new Ice.EndpointParseException();
                    }

                    try
                    {
                        _timeout = Integer.parseInt(argument);
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
    TcpEndpoint(Ice.Stream s)
    {
        s.startReadEncaps();
        _host = s.readString();
        _port = s.readInt();
        _timeout = s.readInt();
        s.endReadEncaps();
    }

    //
    // Marshal the endpoint
    //
    public void
    streamWrite(Ice.Stream s)
    {
        s.writeShort(TcpEndpointType);
        s.startWriteEncaps();
        s.writeString(_host);
        s.writeInt(_port);
        s.writeInt(_timeout);
        s.endWriteEncaps();
    }

    //
    // Convert the endpoint to its string form
    //
    public String
    toString()
    {
        StringBuffer s = new StringBuffer();
        s.append("tcp");
        // TODO: Whether numeric or not should be configurable
        if (!_host.equals(Network.getLocalHost(true)))
        {
            s.append(" -h " + _host);
        }
        if (_port != 0)
        {
            s.append(" -p " + _port);
        }
        if (_timeout != -1)
        {
            s.append(" -t " + _timeout);
        }
        return s.toString();
    }

    //
    // Return the endpoint type
    //
    public short
    type()
    {
        return TcpEndpointType;
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
        return _timeout;
    }
    
    //
    // Return a new endpoint with a different timeout value, provided
    // that timeouts are supported by the endpoint. Otherwise the same
    // endpoint is returned.
    //
    public Endpoint
    timeout(int timeout)
    {
        if (timeout == _timeout)
        {
            return this;
        }
        else
        {
            return new TcpEndpoint(_host, _port, timeout);
        }
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
        return new TcpConnector(instance, _host, _port);
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
        TcpAcceptor p = new TcpAcceptor(instance, _port);
        endpoint.value = new TcpEndpoint(_host, p.effectivePort(), _timeout);
        return p;
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
        TcpAcceptor tcpAcceptor = null;
        try
        {
            tcpAcceptor = (TcpAcceptor)acceptor;
        }
        catch (ClassCastException ex)
        {
            return false;
        }
        return tcpAcceptor.equivalent(_host, _port);
    }
    
    //
    // Compare endpoints for sorting purposes
    //
    public boolean
    equals(java.lang.Object obj)
    {
        TcpEndpoint p = null;

        try
        {
            p = (TcpEndpoint)obj;
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

        if (_timeout != p._timeout)
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
    private int _timeout;
}
