// Copyright (c) ZeroC, Inc.

using System.Net;

namespace Ice.Internal;

public class ProtocolInstance
{
    public ProtocolInstance(Ice.Communicator communicator, short type, string protocol, bool secure)
    {
        instance_ = communicator.instance;
        traceLevel_ = instance_.traceLevels().network;
        traceCategory_ = instance_.traceLevels().networkCat;
        logger_ = instance_.initializationData().logger;
        properties_ = instance_.initializationData().properties;
        type_ = type;
        protocol_ = protocol;
        secure_ = secure;
    }

    public ProtocolInstance(Instance instance, short type, string protocol, bool secure)
    {
        instance_ = instance;
        traceLevel_ = instance_.traceLevels().network;
        traceCategory_ = instance_.traceLevels().networkCat;
        logger_ = instance_.initializationData().logger;
        properties_ = instance_.initializationData().properties;
        type_ = type;
        protocol_ = protocol;
        secure_ = secure;
    }

    public int traceLevel()
    {
        return traceLevel_;
    }

    public string traceCategory()
    {
        return traceCategory_;
    }

    public Ice.Logger logger()
    {
        return logger_;
    }

    public EndpointFactory getEndpointFactory(short type)
    {
        return instance_.endpointFactoryManager().get(type);
    }

    public string protocol()
    {
        return protocol_;
    }

    public short type()
    {
        return type_;
    }

    public bool secure()
    {
        return secure_;
    }

    public Ice.Properties properties()
    {
        return properties_;
    }

    public bool preferIPv6()
    {
        return instance_.preferIPv6();
    }

    public int protocolSupport()
    {
        return instance_.protocolSupport();
    }

    public string defaultHost()
    {
        return instance_.defaultsAndOverrides().defaultHost;
    }

    public EndPoint defaultSourceAddress()
    {
        return instance_.defaultsAndOverrides().defaultSourceAddress;
    }

    public Ice.EncodingVersion defaultEncoding()
    {
        return instance_.defaultsAndOverrides().defaultEncoding;
    }

    public NetworkProxy networkProxy()
    {
        return instance_.networkProxy();
    }

    public int messageSizeMax()
    {
        return instance_.messageSizeMax();
    }

    public void resolve(string host, int port, IPEndpointI endpt, EndpointI_connectors callback)
    {
        instance_.endpointHostResolver().resolve(host, port, endpt, callback);
    }

    public BufSizeWarnInfo getBufSizeWarn(short type)
    {
        return instance_.getBufSizeWarn(type);
    }

    public void setSndBufSizeWarn(short type, int size)
    {
        instance_.setSndBufSizeWarn(type, size);
    }

    public void setRcvBufSizeWarn(short type, int size)
    {
        instance_.setRcvBufSizeWarn(type, size);
    }

    protected readonly Instance instance_;
    protected int traceLevel_;
    protected string traceCategory_;
    protected Ice.Logger logger_;
    protected Ice.Properties properties_;
    protected string protocol_;
    protected short type_;
    protected bool secure_;
}
