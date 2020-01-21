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
            traceLevel_ = communicator_.TraceLevels.network;
            traceCategory_ = communicator_.TraceLevels.networkCat;
            logger_ = communicator_.Logger;
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

        public Ice.ILogger logger()
        {
            return logger_;
        }

        public IEndpointFactory? getEndpointFactory(short type) => communicator_.EndpointFactoryManager().get(type);

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

        public Ice.Communicator communicator()
        {
            return communicator_;
        }

        public bool preferIPv6() => communicator_.PreferIPv6;

        public int protocolSupport() => communicator_.ProtocolSupport;

        public string defaultHost() => communicator_.DefaultsAndOverrides.defaultHost ?? "";

        public EndPoint? defaultSourceAddress() => communicator_.DefaultsAndOverrides.defaultSourceAddress;

        public Ice.EncodingVersion defaultEncoding() => communicator_.DefaultsAndOverrides.defaultEncoding;

        public int defaultTimeout() => communicator_.DefaultsAndOverrides.defaultTimeout;

        public int messageSizeMax() => communicator_.MessageSizeMax;

        public INetworkProxy? networkProxy() => communicator_.NetworkProxy;

        public void resolve(string host, int port, Ice.EndpointSelectionType type, IPEndpoint endpt,
                            IEndpointConnectors callback)
        {
            communicator_.EndpointHostResolver().resolve(host, port, type, endpt, callback);
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
        protected Ice.ILogger logger_;
        protected string protocol_;
        protected short type_;
        protected bool secure_;
    }

}
