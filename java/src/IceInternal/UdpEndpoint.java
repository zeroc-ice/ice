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

final class UdpEndpoint implements Endpoint
{
    final static short TYPE = 3;

    public
    UdpEndpoint(Instance instance, String ho, int po)
    {
        _instance = instance;
        _host = ho;
        _port = po;
        _connect = false;
        calcHashValue();
    }

    public
    UdpEndpoint(Instance instance, String str)
    {
        _instance = instance;
        _host = null;
        _port = 0;
        _connect = false;

        String[] arr = str.split("[ \t\n\r]+");

        int i = 0;
        while (i < arr.length)
        {
            if (arr[i].length() == 0)
            {
                i++;
                continue;
            }

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

                case 'c':
                {
                    if (argument != null)
                    {
                        throw new Ice.EndpointParseException();
                    }

                    _connect = true;
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
            _host = instance.defaultsAndOverwrites().defaultHost;
        }

        calcHashValue();
    }

    public
    UdpEndpoint(BasicStream s)
    {
        _instance = s.instance();
        s.startReadEncaps();
        _host = s.readString();
        _port = s.readInt();
        // Not transmitted.
        //_connect = s.readBool();
        _connect = false;
        s.endReadEncaps();
        calcHashValue();
    }

    //
    // Marshal the endpoint
    //
    public void
    streamWrite(BasicStream s)
    {
        s.writeShort(TYPE);
        s.startWriteEncaps();
        s.writeString(_host);
        s.writeInt(_port);
        // Not transmitted.
        //s.writeBool(_connect);
        s.endWriteEncaps();
    }

    //
    // Convert the endpoint to its string form
    //
    public String
    toString()
    {
        String s = "udp -h " + _host + " -p " + _port;
        if (_connect)
        {
            s += " -c";
        }
        return s;
    }

    //
    // Return the endpoint type
    //
    public short
    type()
    {
        return TYPE;
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
    // Return true if the endpoint type is unknown.
    //
    public boolean
    unknown()
    {
        return false;
    }

    //
    // Return a client side transceiver for this endpoint, or null if a
    // transceiver can only be created by a connector.
    //
    public Transceiver
    clientTransceiver()
    {
        return new UdpTransceiver(_instance, _host, _port);
    }

    //
    // Return a server side transceiver for this endpoint, or null if a
    // transceiver can only be created by an acceptor. In case a
    // transceiver is created, this operation also returns a new
    // "effective" endpoint, which might differ from this endpoint,
    // for example, if a dynamic port number is assigned.
    //
    public Transceiver
    serverTransceiver(EndpointHolder endpoint)
    {
        UdpTransceiver p = new UdpTransceiver(_instance, _host, _port, _connect);
        endpoint.value = new UdpEndpoint(_instance, _host, p.effectivePort());
        return p;
    }

    //
    // Return a connector for this endpoint, or null if no connector
    // is available.
    //
    public Connector
    connector()
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
    acceptor(EndpointHolder endpoint)
    {
        endpoint.value = this;
        return null;
    }

    //
    // Check whether the endpoint is equivalent to a specific
    // Transceiver or Acceptor
    //
    public boolean
    equivalent(Transceiver transceiver)
    {
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
    }

    public boolean
    equivalent(Acceptor acceptor)
    {
        return false;
    }

    public int
    hashCode()
    {
        return _hashCode;
    }

    //
    // Compare endpoints for sorting purposes
    //
    public boolean
    equals(java.lang.Object obj)
    {
        return compareTo(obj) == 0;
    }

    public int
    compareTo(java.lang.Object obj) // From java.lang.Comparable
    {
        UdpEndpoint p = null;

        try
        {
            p = (UdpEndpoint)obj;
        }
        catch (ClassCastException ex)
        {
            return 1;
        }

        if (this == p)
        {
            return 0;
        }

        if (_port < p._port)
        {
            return -1;
        }
        else if (p._port < _port)
        {
            return 1;
        }

        if (!_connect && p._connect)
        {
            return -1;
        }
        else if (!p._connect && _connect)
        {
            return 1;
        }

        if (!_host.equals(p._host))
        {
            //
            // We do the most time-consuming part of the comparison last.
            //
            java.net.InetSocketAddress laddr;
            java.net.InetSocketAddress raddr;
            laddr = Network.getAddress(_host, _port);
            raddr = Network.getAddress(p._host, p._port);
            byte[] larr = laddr.getAddress().getAddress();
            byte[] rarr = raddr.getAddress().getAddress();
            assert(larr.length == rarr.length);
            for (int i = 0; i < larr.length; i++)
            {
                if (larr[i] < rarr[i])
                {
                    return -1;
                }
                else if (rarr[i] < larr[i])
                {
                    return 1;
                }
            }
        }

        return 0;
    }

    private void
    calcHashValue()
    {
        _hashCode = _host.hashCode();
        _hashCode = 5 * _hashCode + _port;
        _hashCode = 5 * _hashCode + (_connect ? 1 : 0);
    }

    private Instance _instance;
    private String _host;
    private int _port;
    private boolean _connect;
    private int _hashCode;
}
