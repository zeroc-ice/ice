//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Net;

namespace IceInternal
{
    public class TransportInstance
    {
        public TransportInstance(Ice.Communicator communicator, short type, string transport, bool secure)
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
        public short Type { get; protected set; }
        public bool Secure { get; protected set; }
        public Ice.Communicator Communicator { get; protected set; }
        public bool PreferIPv6 => Communicator.PreferIPv6;
        public int IPVersion => Communicator.IPVersion;
        public string DefaultHost => Communicator.DefaultsAndOverrides.DefaultHost ?? "";
        public EndPoint? DefaultSourceAddress => Communicator.DefaultsAndOverrides.DefaultSourceAddress;
        public Ice.Encoding DefaultEncoding => Communicator.DefaultsAndOverrides.DefaultEncoding;
        public int DefaultTimeout => Communicator.DefaultsAndOverrides.DefaultTimeout;
        public int MessageSizeMax => Communicator.MessageSizeMax;
        public INetworkProxy? NetworkProxy => Communicator.NetworkProxy;

        public IEndpointFactory? GetEndpointFactory(short type) => Communicator.GetEndpointFactory(type);
        public void Resolve(string host, int port, Ice.EndpointSelectionType type, IPEndpoint endpt,
                            IEndpointConnectors callback) =>
            Communicator.Resolve(host, port, type, endpt, callback);
        public void SetSndBufSizeWarn(short type, int size) => Communicator.SetSndBufSizeWarn(type, size);
        public void SetRcvBufSizeWarn(short type, int size) => Communicator.SetRcvBufSizeWarn(type, size);

        internal Ice.BufSizeWarnInfo GetBufSizeWarn(short type) => Communicator.GetBufSizeWarn(type);
    }
}
