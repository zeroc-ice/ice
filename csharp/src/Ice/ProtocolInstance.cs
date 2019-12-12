//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Net;

namespace IceInternal
{
    public class ProtocolInstance
    {
        public ProtocolInstance(Ice.Communicator communicator, short type, string protocol, bool secure)
        {
            communicator_ = communicator;
            traceLevel_ = communicator_.traceLevels().network;
            traceCategory_ = communicator_.traceLevels().networkCat;
            logger_ = communicator_.initializationData().logger;
            properties_ = communicator_.Properties;
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
            return communicator_.endpointFactoryManager().get(type);
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
            return communicator_.PreferIPv6;
        }

        public int protocolSupport()
        {
            return communicator_.ProtocolSupport;
        }

        public string defaultHost()
        {
            return communicator_.defaultsAndOverrides().defaultHost;
        }

        public EndPoint defaultSourceAddress()
        {
            return communicator_.defaultsAndOverrides().defaultSourceAddress;
        }

        public Ice.EncodingVersion defaultEncoding()
        {
            return communicator_.defaultsAndOverrides().defaultEncoding;
        }

        public int defaultTimeout()
        {
            return communicator_.defaultsAndOverrides().defaultTimeout;
        }

        public NetworkProxy? networkProxy()
        {
            return communicator_.NetworkProxy;
        }

        public int messageSizeMax()
        {
            return communicator_.messageSizeMax();
        }

        public void resolve(string host, int port, Ice.EndpointSelectionType type, IPEndpointI endpt,
                            EndpointI_connectors callback)
        {
            communicator_.endpointHostResolver().resolve(host, port, type, endpt, callback);
        }

        internal Ice.BufSizeWarnInfo getBufSizeWarn(short type)
        {
            return communicator_.GetBufSizeWarn(type);
        }

        public void setSndBufSizeWarn(short type, int size)
        {
            communicator_.SetSndBufSizeWarn(type, size);
        }

        public void setRcvBufSizeWarn(short type, int size)
        {
            communicator_.SetRcvBufSizeWarn(type, size);
        }

        protected Ice.Communicator communicator_;
        protected int traceLevel_;
        protected string traceCategory_;
        protected Ice.Logger logger_;
        protected Ice.Properties properties_;
        protected string protocol_;
        protected short type_;
        protected bool secure_;
    }

}
