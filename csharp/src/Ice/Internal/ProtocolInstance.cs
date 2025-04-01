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

    public int traceLevel() => traceLevel_;

    public string traceCategory() => traceCategory_;

    public Ice.Logger logger() => logger_;

    public EndpointFactory getEndpointFactory(short type) => instance_.endpointFactoryManager().get(type);

    public string protocol() => protocol_;

    public short type() => type_;

    public bool secure() => secure_;

    public Ice.Properties properties() => properties_;

    public bool preferIPv6() => instance_.preferIPv6();

    public int protocolSupport() => instance_.protocolSupport();

    public string defaultHost() => instance_.defaultsAndOverrides().defaultHost;

    public EndPoint defaultSourceAddress() => instance_.defaultsAndOverrides().defaultSourceAddress;

    public Ice.EncodingVersion defaultEncoding() => instance_.defaultsAndOverrides().defaultEncoding;

    public NetworkProxy networkProxy() => instance_.networkProxy();

    public int messageSizeMax() => instance_.messageSizeMax();

    public void resolve(string host, int port, IPEndpointI endpt, EndpointI_connectors callback) =>
        instance_.endpointHostResolver().resolve(host, port, endpt, callback);

    public BufSizeWarnInfo getBufSizeWarn(short type) => instance_.getBufSizeWarn(type);

    public void setSndBufSizeWarn(short type, int size) => instance_.setSndBufSizeWarn(type, size);

    public void setRcvBufSizeWarn(short type, int size) => instance_.setRcvBufSizeWarn(type, size);

    protected readonly Instance instance_;
    protected int traceLevel_;
    protected string traceCategory_;
    protected Ice.Logger logger_;
    protected Ice.Properties properties_;
    protected string protocol_;
    protected short type_;
    protected bool secure_;
}
