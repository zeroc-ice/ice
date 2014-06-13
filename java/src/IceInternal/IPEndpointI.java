// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public abstract class IPEndpointI extends EndpointI
{
    protected IPEndpointI(ProtocolInstance instance, String host, int port, String connectionId)
    {
        _instance = instance;
        _host = host;
        _port = port;
        _connectionId = connectionId;
        _hashInitialized = false;
    }

    protected IPEndpointI(ProtocolInstance instance)
    {
        _instance = instance;
        _host = null;
        _port = 0;
        _connectionId = "";
        _hashInitialized = false;
    }

    protected IPEndpointI(ProtocolInstance instance, BasicStream s)
    {
        _instance = instance;
        _host = s.readString();
        _port = s.readInt();
        _connectionId = "";
        _hashInitialized = false;
    }

    public void streamWrite(BasicStream s)
    {
        s.startWriteEncaps();
        streamWriteImpl(s);
        s.endWriteEncaps();
    }

    public Ice.EndpointInfo getInfo()
    {
        Ice.IPEndpointInfo info = new Ice.IPEndpointInfo()
            {
                public short type()
                {
                    return IPEndpointI.this.type();
                }

                public boolean datagram()
                {
                    return IPEndpointI.this.datagram();
                }

                public boolean secure()
                {
                    return IPEndpointI.this.secure();
                }
        };
        fillEndpointInfo(info);
        return info;
    }

    public short type()
    {
        return _instance.type();
    }

    public String protocol()
    {
        return _instance.protocol();
    }

    public String connectionId()
    {
        return _connectionId;
    }

    public EndpointI connectionId(String connectionId)
    {
        if(connectionId.equals(_connectionId))
        {
            return this;
        }
        else
        {
            return createEndpoint(_host, _port, connectionId);
        }
    }

    public java.util.List<Connector> connectors(Ice.EndpointSelectionType selType)
    {
        return _instance.resolve(_host, _port, selType, this);
    }

    public void connectors_async(Ice.EndpointSelectionType selType, EndpointI_connectors callback)
    {
        _instance.resolve(_host, _port, selType, this, callback);
    }

    public java.util.List<EndpointI> expand()
    {
        java.util.List<EndpointI> endps = new java.util.ArrayList<EndpointI>();
        java.util.List<String> hosts = Network.getHostsForEndpointExpand(_host, _instance.protocolSupport(), false);
        if(hosts == null || hosts.isEmpty())
        {
            endps.add(this);
        }
        else
        {
            for(String h : hosts)
            {
                endps.add(createEndpoint(h, _port, _connectionId));
            }
        }
        return endps;
    }

    public boolean equivalent(EndpointI endpoint)
    {
        if(!(endpoint instanceof IPEndpointI))
        {
            return false;
        }
        IPEndpointI ipEndpointI = (IPEndpointI)endpoint;
        return ipEndpointI.type() == type() && ipEndpointI._host.equals(_host) && ipEndpointI._port == _port;
    }

    public java.util.List<Connector> connectors(java.util.List<java.net.InetSocketAddress> addresses,
                                                NetworkProxy proxy)
    {
        java.util.List<Connector> connectors = new java.util.ArrayList<Connector>();
        for(java.net.InetSocketAddress p : addresses)
        {
            connectors.add(createConnector(p, proxy));
        }
        return connectors;
    }

    synchronized public int hashCode()
    {
        if(!_hashInitialized)
        {
            _hashValue = 5381;
            _hashValue = HashUtil.hashAdd(_hashValue, type());
            _hashValue = hashInit(_hashValue);
            _hashInitialized = true;
        }
        return _hashValue;
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
        String s = "";

        if(_host != null && _host.length() > 0)
        {
            s += " -h ";
            boolean addQuote = _host.indexOf(':') != -1;
            if(addQuote)
            {
                s += "\"";
            }
            s += _host;
            if(addQuote)
            {
                s += "\"";
            }
        }

        s += " -p " + _port;

        return s;
    }

    public int compareTo(EndpointI obj) // From java.lang.Comparable
    {
        if(!(obj instanceof IPEndpointI))
        {
            return type() < obj.type() ? -1 : 1;
        }

        IPEndpointI p = (IPEndpointI)obj;
        if(this == p)
        {
            return 0;
        }

        int v = _host.compareTo(p._host);
        if(v != 0)
        {
            return v;
        }

        if(_port < p._port)
        {
            return -1;
        }
        else if(p._port < _port)
        {
            return 1;
        }

        return _connectionId.compareTo(p._connectionId);
    }

    public String host()
    {
        return _host;
    }

    public int port()
    {
        return _port;
    }

    protected void streamWriteImpl(BasicStream s)
    {
        s.writeString(_host);
        s.writeInt(_port);
    }

    protected int hashInit(int h)
    {
        h = HashUtil.hashAdd(h, _host);
        h = HashUtil.hashAdd(h, _port);
        h = HashUtil.hashAdd(h, _connectionId);
        return h;
    }

    protected void fillEndpointInfo(Ice.IPEndpointInfo info)
    {
        info.host = _host;
        info.port = _port;
    }

    public void initWithOptions(java.util.ArrayList<String> args, boolean oaEndpoint)
    {
        super.initWithOptions(args);

        if(_host == null || _host.length() == 0)
        {
            _host = _instance.defaultHost();
        }
        else if(_host.equals("*"))
        {
            if(oaEndpoint)
            {
                _host = "";
            }
            else
            {
                throw new Ice.EndpointParseException("`-h *' not valid for proxy endpoint `" + toString() + "'");
            }
        }
    }

    protected boolean checkOption(String option, String argument, String endpoint)
    {
        switch(option.charAt(1))
        {
            case 'h':
            {
                if(argument == null)
                {
                    throw new Ice.EndpointParseException("no argument provided for -h option in endpoint " + endpoint);
                }
                _host = argument;
                return true;
            }

            case 'p':
            {
                if(argument == null)
                {
                    throw new Ice.EndpointParseException("no argument provided for -p option in endpoint " + endpoint);
                }

                try
                {
                    _port = Integer.parseInt(argument);
                }
                catch(NumberFormatException ex)
                {
                    throw new Ice.EndpointParseException("invalid port value `" + argument +
                                                         "' in endpoint " + endpoint);
                }

                if(_port < 0 || _port > 65535)
                {
                    throw new Ice.EndpointParseException("port value `" + argument +
                                                         "' out of range in endpoint " + endpoint);
                }

                return true;
            }

            default:
            {
                return false;
            }
        }
    }

    protected abstract Connector createConnector(java.net.InetSocketAddress addr, NetworkProxy proxy);
    protected abstract IPEndpointI createEndpoint(String host, int port, String connectionId);

    protected ProtocolInstance _instance;
    protected String _host;
    protected int _port;
    protected String _connectionId;
    private boolean _hashInitialized;
    private int _hashValue;
}
