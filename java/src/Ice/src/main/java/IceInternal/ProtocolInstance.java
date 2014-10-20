// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class ProtocolInstance
{
    public ProtocolInstance(Ice.Communicator communicator, short type, String protocol)
    {
        _instance = Util.getInstance(communicator);
        _traceLevel = _instance.traceLevels().network;
        _traceCategory = _instance.traceLevels().networkCat;
        _logger = _instance.initializationData().logger;
        _properties = _instance.initializationData().properties;
        _type = type;
        _protocol = protocol;
    }

    public int traceLevel()
    {
        return _traceLevel;
    }

    public String traceCategory()
    {
        return _traceCategory;
    }

    public Ice.Logger logger()
    {
        return _logger;
    }

    public String protocol()
    {
        return _protocol;
    }

    public short type()
    {
        return _type;
    }

    public Ice.Properties properties()
    {
        return _properties;
    }

    public boolean preferIPv6()
    {
        return _instance.preferIPv6();
    }

    public int protocolSupport()
    {
        return _instance.protocolSupport();
    }

    public String defaultHost()
    {
        return _instance.defaultsAndOverrides().defaultHost;
    }

    public java.net.InetSocketAddress defaultSourceAddress()
    {
        return _instance.defaultsAndOverrides().defaultSourceAddress;
    }

    public Ice.EncodingVersion defaultEncoding()
    {
        return _instance.defaultsAndOverrides().defaultEncoding;
    }

    public int defaultTimeout()
    {
        return _instance.defaultsAndOverrides().defaultTimeout;
    }

    public NetworkProxy networkProxy()
    {
        return _instance.networkProxy();
    }

    public int messageSizeMax()
    {
        return _instance.messageSizeMax();
    }

    public java.util.List<Connector> resolve(String host, int port, Ice.EndpointSelectionType type, IPEndpointI endpt)
    {
        return _instance.endpointHostResolver().resolve(host, port, type, endpt);
    }

    public void resolve(String host, int port, Ice.EndpointSelectionType type, IPEndpointI endpt,
                        EndpointI_connectors callback)
    {
        _instance.endpointHostResolver().resolve(host, port, type, endpt, callback);
    }

    ProtocolInstance(Instance instance, short type, String protocol)
    {
        _instance = instance;
        _traceLevel = _instance.traceLevels().network;
        _traceCategory = _instance.traceLevels().networkCat;
        _logger = _instance.initializationData().logger;
        _properties = _instance.initializationData().properties;
        _type = type;
        _protocol = protocol;
    }

    protected Instance _instance;
    protected int _traceLevel;
    protected String _traceCategory;
    protected Ice.Logger _logger;
    protected Ice.Properties _properties;
    protected String _protocol;
    protected short _type;
}
