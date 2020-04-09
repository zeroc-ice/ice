//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using System.Net;

namespace IceInternal
{
    public class TransportInstance
    {
        public TransportInstance(Ice.Communicator communicator, EndpointType type, string transport, bool secure)
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
        public Ice.ILogger Logger { get; protected set; }
        public string Transport { get; protected set; }
        public EndpointType Type { get; protected set; }
        public bool Secure { get; protected set; }
        public Ice.Communicator Communicator { get; protected set; }
        public bool PreferIPv6 => Communicator.PreferIPv6;
        public int IPVersion => Communicator.IPVersion;
        public string DefaultHost => Communicator.DefaultsAndOverrides.DefaultHost ?? "";
        public IPAddress? DefaultSourceAddress => Communicator.DefaultsAndOverrides.DefaultSourceAddress;
        public Ice.Encoding DefaultEncoding => Communicator.DefaultsAndOverrides.DefaultEncoding;
        public int DefaultTimeout => Communicator.DefaultsAndOverrides.DefaultTimeout;
        public int MessageSizeMax => Communicator.MessageSizeMax;
        public INetworkProxy? NetworkProxy => Communicator.NetworkProxy;

        public IEndpointFactory? GetEndpointFactory(EndpointType type) => Communicator.GetEndpointFactory(type);
        public void Resolve(string host, int port, Ice.EndpointSelectionType type, IPEndpoint endpt,
                            IEndpointConnectors callback) =>
            Communicator.Resolve(host, port, type, endpt, callback);
        public void SetSndBufSizeWarn(EndpointType type, int size) => Communicator.SetSndBufSizeWarn(type, size);
        public void SetRcvBufSizeWarn(EndpointType type, int size) => Communicator.SetRcvBufSizeWarn(type, size);

        internal Ice.BufSizeWarnInfo GetBufSizeWarn(EndpointType type) => Communicator.GetBufSizeWarn(type);
    }
}
