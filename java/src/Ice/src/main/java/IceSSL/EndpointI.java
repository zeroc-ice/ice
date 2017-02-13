// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

final class EndpointI extends IceInternal.IPEndpointI implements IceInternal.WSEndpointDelegate
{
    public EndpointI(Instance instance, String ho, int po, java.net.InetSocketAddress sourceAddr, int ti, String conId,
                     boolean co)
    {
        super(instance, ho, po, sourceAddr, conId);
        _instance = instance;
        _timeout = ti;
        _compress = co;
    }

    public EndpointI(Instance instance)
    {
        super(instance);
        _instance = instance;
        _timeout = instance.defaultTimeout();
        _compress = false;
    }

    public EndpointI(Instance instance, IceInternal.BasicStream s)
    {
        super(instance, s);
        _instance = instance;
        _timeout = s.readInt();
        _compress = s.readBool();
    }

    //
    // Return the endpoint information.
    //
    @Override
    public Ice.EndpointInfo getInfo()
    {
        Ice.IPEndpointInfo info = new IceSSL.EndpointInfo()
        {
            @Override
            public short type()
            {
                return EndpointI.this.type();
            }

            @Override
            public boolean datagram()
            {
                return EndpointI.this.datagram();
            }

            @Override
            public boolean secure()
            {
                return EndpointI.this.secure();
            }
        };
        fillEndpointInfo(info);
        return info;
    }

    //
    // Return the secure WebSocket endpoint information.
    //
    @Override
    public Ice.EndpointInfo getWSInfo(String resource)
    {
        IceSSL.WSSEndpointInfo info = new IceSSL.WSSEndpointInfo()
        {
            @Override
            public short type()
            {
                return EndpointI.this.type();
            }

            @Override
            public boolean datagram()
            {
                return EndpointI.this.datagram();
            }

            @Override
            public boolean secure()
            {
                return EndpointI.this.secure();
            }
        };
        fillEndpointInfo(info);
        info.resource = resource;
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
    public IceInternal.EndpointI timeout(int timeout)
    {
        if(timeout == _timeout)
        {
            return this;
        }
        else
        {
            return new EndpointI(_instance, _host, _port, _sourceAddr, timeout, _connectionId, _compress);
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
    public IceInternal.EndpointI compress(boolean compress)
    {
        if(compress == _compress)
        {
            return this;
        }
        else
        {
            return new EndpointI(_instance, _host, _port, _sourceAddr, _timeout, _connectionId, compress);
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
    public IceInternal.Transceiver transceiver()
    {
        return null;
    }

    //
    // Return an acceptor for this endpoint, or null if no acceptors
    // is available.
    //
    @Override
    public IceInternal.Acceptor acceptor(String adapterName)
    {
        return new AcceptorI(this, _instance, adapterName, _host, _port);
    }

    public EndpointI endpoint(AcceptorI acceptor)
    {
        return new EndpointI(_instance, _host, acceptor.effectivePort(), _sourceAddr, _timeout, _connectionId,
                             _compress);
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
    public int compareTo(IceInternal.EndpointI obj) // From java.lang.Comparable
    {
        if(!(obj instanceof EndpointI))
        {
            return type() < obj.type() ? -1 : 1;
        }

        EndpointI p = (EndpointI)obj;
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
    public void streamWriteImpl(IceInternal.BasicStream s)
    {
        super.streamWriteImpl(s);
        s.writeInt(_timeout);
        s.writeBool(_compress);
    }

    @Override
    public int hashInit(int h)
    {
        h = super.hashInit(h);
        h = IceInternal.HashUtil.hashAdd(h, _timeout);
        h = IceInternal.HashUtil.hashAdd(h, _compress);
        return h;
    }

    @Override
    public void fillEndpointInfo(Ice.IPEndpointInfo info)
    {
        super.fillEndpointInfo(info);
        info.timeout = _timeout;
        info.compress = _compress;
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
                throw new Ice.EndpointParseException("no argument provided for -t option in endpoint " + endpoint);
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
                        throw new Ice.EndpointParseException("invalid timeout value `" + argument +
                                                         "' in endpoint " + endpoint);
                    }
                }
                catch(NumberFormatException ex)
                {
                    throw new Ice.EndpointParseException("invalid timeout value `" + argument +
                                                         "' in endpoint " + endpoint);
                }
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

    @Override
    protected IceInternal.Connector createConnector(java.net.InetSocketAddress addr, IceInternal.NetworkProxy proxy)
    {
        return new ConnectorI(_instance, _host, addr, proxy, _sourceAddr, _timeout, _connectionId);
    }

    @Override
    protected IceInternal.IPEndpointI createEndpoint(String host, int port, String connectionId)
    {
        return new EndpointI(_instance, host, port, _sourceAddr, _timeout, connectionId, _compress);
    }

    private Instance _instance;
    private int _timeout;
    private boolean _compress;
}
