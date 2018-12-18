// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

import com.zeroc.Ice.EndpointSelectionType;

public abstract class IPEndpointI extends EndpointI
{
    protected IPEndpointI(ProtocolInstance instance, String host, int port, java.net.InetSocketAddress sourceAddr,
                          String connectionId)
    {
        _instance = instance;
        _host = host;
        _port = port;
        _sourceAddr = sourceAddr;
        _connectionId = connectionId;
        _hashInitialized = false;
    }

    protected IPEndpointI(ProtocolInstance instance)
    {
        _instance = instance;
        _host = null;
        _port = 0;
        _sourceAddr = null;
        _connectionId = "";
        _hashInitialized = false;
    }

    protected IPEndpointI(ProtocolInstance instance, com.zeroc.Ice.InputStream s)
    {
        _instance = instance;
        _host = s.readString();
        _port = s.readInt();
        _sourceAddr = null;
        _connectionId = "";
        _hashInitialized = false;
    }

    @Override
    public com.zeroc.Ice.EndpointInfo getInfo()
    {
        com.zeroc.Ice.IPEndpointInfo info = new com.zeroc.Ice.IPEndpointInfo()
            {
                @Override
                public short type()
                {
                    return IPEndpointI.this.type();
                }

                @Override
                public boolean datagram()
                {
                    return IPEndpointI.this.datagram();
                }

                @Override
                public boolean secure()
                {
                    return IPEndpointI.this.secure();
                }
        };
        fillEndpointInfo(info);
        return info;
    }

    @Override
    public short type()
    {
        return _instance.type();
    }

    @Override
    public String protocol()
    {
        return _instance.protocol();
    }

    @Override
    public boolean secure()
    {
        return _instance.secure();
    }

    @Override
    public String connectionId()
    {
        return _connectionId;
    }

    @Override
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

    @Override
    public void connectors_async(com.zeroc.Ice.EndpointSelectionType selType, EndpointI_connectors callback)
    {
        _instance.resolve(_host, _port, selType, this, callback);
    }

    @Override
    public java.util.List<EndpointI> expandIfWildcard()
    {
        java.util.List<EndpointI> endps = new java.util.ArrayList<>();
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

    @Override
    public EndpointI.ExpandHostResult expandHost()
    {
        EndpointI.ExpandHostResult result = new EndpointI.ExpandHostResult();

        //
        // If this endpoint has an empty host (wildcard address), don't expand, just return
        // this endpoint.
        //
        if(_host.isEmpty())
        {
            result.endpoints = new java.util.ArrayList<>();
            result.endpoints.add(this);
            return result;
        }

        //
        // If using a fixed port, this endpoint can be used as the published endpoint to
        // access the returned endpoints. Otherwise, we'll publish each individual expanded
        // endpoint.
        //
        result.publish = _port > 0 ? this : null;

        java.util.List<java.net.InetSocketAddress> addresses = Network.getAddresses(_host,
                                                                                    _port,
                                                                                    _instance.protocolSupport(),
                                                                                    EndpointSelectionType.Ordered,
                                                                                    _instance.preferIPv6(),
                                                                                    true);

        result.endpoints = new java.util.ArrayList<>();
        if(addresses.size() == 1)
        {
            result.endpoints.add(this);
        }
        else
        {
            for(java.net.InetSocketAddress addr : addresses)
            {
                result.endpoints.add(createEndpoint(addr.getAddress().getHostAddress(), addr.getPort(), _connectionId));
            }
        }
        return result;
    }

    @Override
    public boolean equivalent(EndpointI endpoint)
    {
        if(!(endpoint instanceof IPEndpointI))
        {
            return false;
        }
        IPEndpointI ipEndpointI = (IPEndpointI)endpoint;
        return ipEndpointI.type() == type() && ipEndpointI._host.equals(_host) && ipEndpointI._port == _port &&
            Network.compareAddress(ipEndpointI._sourceAddr, _sourceAddr) == 0;
    }

    public java.util.List<Connector> connectors(java.util.List<java.net.InetSocketAddress> addresses,
                                                NetworkProxy proxy)
    {
        java.util.List<Connector> connectors = new java.util.ArrayList<>();
        for(java.net.InetSocketAddress p : addresses)
        {
            connectors.add(createConnector(p, proxy));
        }
        return connectors;
    }

    @Override
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

        if(_sourceAddr != null)
        {
            s += " --sourceAddress " + _sourceAddr.getAddress().getHostAddress();
        }

        return s;
    }

    @Override
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

        int rc = Network.compareAddress(_sourceAddr, p._sourceAddr);
        if(rc != 0)
        {
            return rc;
        }

        return _connectionId.compareTo(p._connectionId);
    }

    @Override
    public void streamWriteImpl(com.zeroc.Ice.OutputStream s)
    {
        s.writeString(_host);
        s.writeInt(_port);
    }

    public int hashInit(int h)
    {
        h = HashUtil.hashAdd(h, _host);
        h = HashUtil.hashAdd(h, _port);
        if(_sourceAddr != null)
        {
            h = HashUtil.hashAdd(h, _sourceAddr.getAddress().getHostAddress());
        }
        h = HashUtil.hashAdd(h, _connectionId);
        return h;
    }

    public void fillEndpointInfo(com.zeroc.Ice.IPEndpointInfo info)
    {
        info.timeout = timeout();
        info.compress = compress();
        info.host = _host;
        info.port = _port;
        info.sourceAddress = _sourceAddr == null ? "" : _sourceAddr.getAddress().getHostAddress();
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
                throw new com.zeroc.Ice.EndpointParseException(
                    "`-h *' not valid for proxy endpoint `" + toString() + "'");
            }
        }

        if(_host == null)
        {
            _host = "";
        }

        if(_sourceAddr == null)
        {
            if (!oaEndpoint)
            {
                _sourceAddr = _instance.defaultSourceAddress();
            }
        }
        else if(oaEndpoint)
        {
            throw new com.zeroc.Ice.EndpointParseException(
                "`--sourceAddress' not valid for object adapter endpoint `" + toString() + "'");
        }
    }

    @Override
    protected boolean checkOption(String option, String argument, String endpoint)
    {
        if(option.equals("-h"))
        {
            if(argument == null)
            {
                throw new com.zeroc.Ice.EndpointParseException(
                    "no argument provided for -h option in endpoint " + endpoint);
            }
            _host = argument;
        }
        else if(option.equals("-p"))
        {
            if(argument == null)
            {
                throw new com.zeroc.Ice.EndpointParseException(
                    "no argument provided for -p option in endpoint " + endpoint);
            }

            try
            {
                _port = Integer.parseInt(argument);
            }
            catch(NumberFormatException ex)
            {
                throw new com.zeroc.Ice.EndpointParseException(
                    "invalid port value `" + argument + "' in endpoint " + endpoint);
            }

            if(_port < 0 || _port > 65535)
            {
                throw new com.zeroc.Ice.EndpointParseException(
                    "port value `" + argument + "' out of range in endpoint " + endpoint);
            }
        }
        else if(option.equals("--sourceAddress"))
        {
            if(argument == null)
            {
                throw new com.zeroc.Ice.EndpointParseException(
                    "no argument provided for --sourceAddress option in endpoint " + endpoint);
            }
            _sourceAddr = Network.getNumericAddress(argument);
            if(_sourceAddr == null)
            {
                throw new com.zeroc.Ice.EndpointParseException(
                    "invalid IP address provided for --sourceAddress option in endpoint " + endpoint);
            }
        }
        else
        {
            return false;
        }
        return true;
    }

    protected abstract Connector createConnector(java.net.InetSocketAddress addr, NetworkProxy proxy);
    protected abstract IPEndpointI createEndpoint(String host, int port, String connectionId);

    protected ProtocolInstance _instance;
    protected String _host;
    protected int _port;
    protected java.net.InetSocketAddress _sourceAddr;
    protected String _connectionId;
    private boolean _hashInitialized;
    private int _hashValue;
}
