// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final class TcpEndpointI extends IPEndpointI
{
    public TcpEndpointI(ProtocolInstance instance, String ho, int po, int ti, String conId, boolean co)
    {
        super(instance, ho, po, conId);
        _timeout = ti;
        _compress = co;
    }

    public TcpEndpointI(ProtocolInstance instance)
    {
        super(instance);
        _timeout = -1;
        _compress = false;
    }

    public TcpEndpointI(ProtocolInstance instance, BasicStream s)
    {
        super(instance, s);
        _timeout = s.readInt();
        _compress = s.readBool();
    }

    //
    // Return the endpoint information.
    //
    public Ice.EndpointInfo getInfo()
    {
        Ice.TCPEndpointInfo info = new Ice.TCPEndpointInfo()
            {
                public short type()
                {
                    return TcpEndpointI.this.type();
                }

                public boolean datagram()
                {
                    return TcpEndpointI.this.datagram();
                }

                public boolean secure()
                {
                    return TcpEndpointI.this.secure();
                }
            };

        fillEndpointInfo(info);
        return info;
    }

    //
    // Return the timeout for the endpoint in milliseconds. 0 means
    // non-blocking, -1 means no timeout.
    //
    public int timeout()
    {
        return _timeout;
    }

    //
    // Return a new endpoint with a different timeout value, provided
    // that timeouts are supported by the endpoint. Otherwise the same
    // endpoint is returned.
    //
    public EndpointI timeout(int timeout)
    {
        if(timeout == _timeout)
        {
            return this;
        }
        else
        {
            return new TcpEndpointI(_instance, _host, _port, timeout, _connectionId, _compress);
        }
    }

    //
    // Return true if the endpoints support bzip2 compress, or false
    // otherwise.
    //
    public boolean compress()
    {
        return _compress;
    }

    //
    // Return a new endpoint with a different compression value,
    // provided that compression is supported by the
    // endpoint. Otherwise the same endpoint is returned.
    //
    public EndpointI compress(boolean compress)
    {
        if(compress == _compress)
        {
            return this;
        }
        else
        {
            return new TcpEndpointI(_instance, _host, _port, _timeout, _connectionId, compress);
        }
    }

    //
    // Return true if the endpoint is datagram-based.
    //
    public boolean datagram()
    {
        return false;
    }

    //
    // Return true if the endpoint is secure.
    //
    public boolean secure()
    {
        return false;
    }

    //
    // Return a server side transceiver for this endpoint, or null if a
    // transceiver can only be created by an acceptor. In case a
    // transceiver is created, this operation also returns a new
    // "effective" endpoint, which might differ from this endpoint,
    // for example, if a dynamic port number is assigned.
    //
    public Transceiver transceiver(EndpointIHolder endpoint)
    {
        endpoint.value = this;
        return null;
    }

    //
    // Return an acceptor for this endpoint, or null if no acceptors
    // is available. In case an acceptor is created, this operation
    // also returns a new "effective" endpoint, which might differ
    // from this endpoint, for example, if a dynamic port number is
    // assigned.
    //
    public Acceptor acceptor(EndpointIHolder endpoint, String adapterName)
    {
        TcpAcceptor p = new TcpAcceptor(_instance, _host, _port);
        endpoint.value = createEndpoint(_host, p.effectivePort(), _connectionId);
        return p;
    }

    public String options()
    {
        //
        // WARNING: Certain features, such as proxy validation in Glacier2,
        // depend on the format of proxy strings. Changes to toString() and
        // methods called to generate parts of the reference string could break
        // these features. Please review for all features that depend on the
        // format of proxyToString() before changing this and related code.
        //
        String s = super.options();

        if(_timeout != -1)
        {
            s += " -t " + _timeout;
        }

        if(_compress)
        {
            s += " -z";
        }

        return s;
    }

    //
    // Compare endpoints for sorting purposes
    //
    public int compareTo(EndpointI obj) // From java.lang.Comparable
    {
        if(!(obj instanceof TcpEndpointI))
        {
            return type() < obj.type() ? -1 : 1;
        }

        TcpEndpointI p = (TcpEndpointI)obj;
        if(this == p)
        {
            return 0;
        }

        if(_timeout < p._timeout)
        {
            return -1;
        }
        else if(p._timeout < _timeout)
        {
            return 1;
        }

        if(!_compress && p._compress)
        {
            return -1;
        }
        else if(!p._compress && _compress)
        {
            return 1;
        }

        return super.compareTo(obj);
    }

    public void streamWriteImpl(BasicStream s)
    {
        super.streamWriteImpl(s);
        s.writeInt(_timeout);
        s.writeBool(_compress);
    }

    public int hashInit(int h)
    {
        h = super.hashInit(h);
        h = IceInternal.HashUtil.hashAdd(h, _timeout);
        h = IceInternal.HashUtil.hashAdd(h, _compress);
        return h;
    }

    public void fillEndpointInfo(Ice.IPEndpointInfo info)
    {
        super.fillEndpointInfo(info);
        if(info instanceof Ice.TCPEndpointInfo)
        {
            Ice.TCPEndpointInfo tcpInfo = (Ice.TCPEndpointInfo)info;
            tcpInfo.timeout = _timeout;
            tcpInfo.compress = _compress;
        }
    }

    protected boolean checkOption(String option, String argument, String endpoint)
    {
        if(super.checkOption(option, argument, endpoint))
        {
            return true;
        }

        switch(option.charAt(1))
        {
            case 't':
            {
                if(argument == null)
                {
                    throw new Ice.EndpointParseException("no argument provided for -t option in endpoint " + endpoint);
                }

                try
                {
                    _timeout = Integer.parseInt(argument);
                }
                catch(NumberFormatException ex)
                {
                    throw new Ice.EndpointParseException("invalid timeout value `" + argument +
                                                         "' in endpoint " + endpoint);
                }

                return true;
            }

            case 'z':
            {
                if(argument != null)
                {
                    throw new Ice.EndpointParseException("unexpected argument `" + argument +
                                                         "' provided for -z option in " + endpoint);
                }

                _compress = true;

                return true;
            }

            default:
            {
                return false;
            }
        }
    }

    protected Connector createConnector(java.net.InetSocketAddress addr, NetworkProxy proxy)
    {
        return new TcpConnector(_instance, addr, proxy, _timeout, _connectionId);
    }

    protected IPEndpointI createEndpoint(String host, int port, String connectionId)
    {
        return new TcpEndpointI(_instance, host, port, _timeout, connectionId, _compress);
    }

    private int _timeout;
    private boolean _compress;
}
