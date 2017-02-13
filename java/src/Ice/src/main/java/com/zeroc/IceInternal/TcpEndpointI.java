// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

final class TcpEndpointI extends IPEndpointI
{
    public TcpEndpointI(ProtocolInstance instance, String ho, int po, java.net.InetSocketAddress sourceAddr, int ti,
                        String conId, boolean co)
    {
        super(instance, ho, po, sourceAddr, conId);
        _timeout = ti;
        _compress = co;
    }

    public TcpEndpointI(ProtocolInstance instance)
    {
        super(instance);
        _timeout = _instance.defaultTimeout();
        _compress = false;
    }

    public TcpEndpointI(ProtocolInstance instance, com.zeroc.Ice.InputStream s)
    {
        super(instance, s);
        _timeout = s.readInt();
        _compress = s.readBool();
    }

    //
    // Return the endpoint information.
    //
    @Override
    public com.zeroc.Ice.EndpointInfo getInfo()
    {
        com.zeroc.Ice.TCPEndpointInfo info = new com.zeroc.Ice.TCPEndpointInfo()
        {
            @Override
            public short type()
            {
                return TcpEndpointI.this.type();
            }

            @Override
            public boolean datagram()
            {
                return TcpEndpointI.this.datagram();
            }

            @Override
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
    @Override
    public int timeout()
    {
        return _timeout;
    }

    //
    // Return a new endpoint with a different timeout value, provided
    // that timeouts are supported by the endpoint. Otherwise the same
    // endpoint is returned.
    //
    @Override
    public EndpointI timeout(int timeout)
    {
        if(timeout == _timeout)
        {
            return this;
        }
        else
        {
            return new TcpEndpointI(_instance, _host, _port, _sourceAddr, timeout, _connectionId, _compress);
        }
    }

    //
    // Return true if the endpoints support bzip2 compress, or false
    // otherwise.
    //
    @Override
    public boolean compress()
    {
        return _compress;
    }

    //
    // Return a new endpoint with a different compression value,
    // provided that compression is supported by the
    // endpoint. Otherwise the same endpoint is returned.
    //
    @Override
    public EndpointI compress(boolean compress)
    {
        if(compress == _compress)
        {
            return this;
        }
        else
        {
            return new TcpEndpointI(_instance, _host, _port, _sourceAddr, _timeout, _connectionId, compress);
        }
    }

    //
    // Return true if the endpoint is datagram-based.
    //
    @Override
    public boolean datagram()
    {
        return false;
    }

    //
    // Return a server side transceiver for this endpoint, or null if a
    // transceiver can only be created by an acceptor.
    //
    @Override
    public Transceiver transceiver()
    {
        return null;
    }

    //
    // Return an acceptor for this endpoint, or null if no acceptors
    // is available.
    //
    @Override
    public Acceptor acceptor(String adapterName)
    {
        return new TcpAcceptor(this, _instance, _host, _port);
    }

    public TcpEndpointI endpoint(TcpAcceptor acceptor)
    {
        return new TcpEndpointI(_instance, _host, acceptor.effectivePort(), _sourceAddr, _timeout,
                                _connectionId, _compress);
    }

    @Override
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

        if(_timeout == -1)
        {
            s += " -t infinite";
        }
        else
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
    @Override
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

    @Override
    public void streamWriteImpl(com.zeroc.Ice.OutputStream s)
    {
        super.streamWriteImpl(s);
        s.writeInt(_timeout);
        s.writeBool(_compress);
    }

    @Override
    public int hashInit(int h)
    {
        h = super.hashInit(h);
        h = HashUtil.hashAdd(h, _timeout);
        h = HashUtil.hashAdd(h, _compress);
        return h;
    }

    @Override
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
                    throw new com.zeroc.Ice.EndpointParseException("no argument provided for -t option in endpoint " +
                                                                   endpoint);
                }

                if(argument.equals("infinite"))
                {
                    _timeout = -1;
                }
                else
                {
                    try
                    {
                        _timeout = Integer.parseInt(argument);
                        if(_timeout < 1)
                        {
                            throw new com.zeroc.Ice.EndpointParseException("invalid timeout value `" + argument +
                                                                           "' in endpoint " + endpoint);
                        }
                    }
                    catch(NumberFormatException ex)
                    {
                        throw new com.zeroc.Ice.EndpointParseException("invalid timeout value `" + argument +
                                                                       "' in endpoint " + endpoint);
                    }
                }

                return true;
            }

            case 'z':
            {
                if(argument != null)
                {
                    throw new com.zeroc.Ice.EndpointParseException("unexpected argument `" + argument +
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

    @Override
    protected Connector createConnector(java.net.InetSocketAddress addr, NetworkProxy proxy)
    {
        return new TcpConnector(_instance, addr, proxy, _sourceAddr, _timeout, _connectionId);
    }

    @Override
    protected IPEndpointI createEndpoint(String host, int port, String connectionId)
    {
        return new TcpEndpointI(_instance, host, port, _sourceAddr, _timeout, connectionId, _compress);
    }

    private int _timeout;
    private boolean _compress;
}
