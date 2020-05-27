//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Net;

namespace ZeroC.Ice
{
    public class TransportInstance
    {
        public TransportInstance(Communicator communicator, EndpointType type, string transport, bool secure)
        {
            Communicator = communicator;
            TraceLevel = Communicator.TraceLevels.Network;
            TraceCategory = Communicator.TraceLevels.NetworkCat;
            Logger = Communicator.Logger;
            Type = type;
            Transport = transport;
            Secure = secure;
        }

        public int TraceLevel { get; protected set; }
        public string TraceCategory { get; protected set; }
        public ILogger Logger { get; protected set; }
        public string Transport { get; protected set; }
        public EndpointType Type { get; protected set; }
        public bool Secure { get; protected set; }
        public Communicator Communicator { get; protected set; }
        public bool PreferIPv6 => Communicator.PreferIPv6;
        public int IPVersion => Communicator.IPVersion;
        public string DefaultHost => Communicator.DefaultHost ?? "";
        public IPAddress? DefaultSourceAddress => Communicator.DefaultSourceAddress;
        public Encoding DefaultEncoding => Communicator.DefaultEncoding;
        public int DefaultTimeout => Communicator.DefaultTimeout;
        public int MessageSizeMax => Communicator.MessageSizeMax;
        public INetworkProxy? NetworkProxy => Communicator.NetworkProxy;

        public IEndpointFactory? GetEndpointFactory(EndpointType type) => Communicator.FindEndpointFactory(type);
        public void SetSndBufSizeWarn(EndpointType type, int size) => Communicator.SetSndBufSizeWarn(type, size);
        public void SetRcvBufSizeWarn(EndpointType type, int size) => Communicator.SetRcvBufSizeWarn(type, size);

        internal BufSizeWarnInfo GetBufSizeWarn(EndpointType type) => Communicator.GetBufSizeWarn(type);
    }
}
