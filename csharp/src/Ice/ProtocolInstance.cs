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
            Communicator = communicator;
            TraceLevel = Communicator.TraceLevels.network;
            TraceCategory = Communicator.TraceLevels.networkCat;
            Logger = Communicator.Logger;
            Type = type;
            Protocol = protocol;
            Secure = secure;
        }

        public int TraceLevel { get; protected set; }
        public string TraceCategory { get; protected set; }
        public Ice.ILogger Logger { get; protected set; }
        public string Protocol { get; protected set; }
        public short Type { get; protected set; }
        public bool Secure { get; protected set; }
        public Ice.Communicator Communicator { get; protected set; }
        public bool PreferIPv6 => Communicator.PreferIPv6;
        public int ProtocolSupport => Communicator.ProtocolSupport;
        public string DefaultHost => Communicator.DefaultsAndOverrides.defaultHost ?? "";
        public EndPoint? DefaultSourceAddress => Communicator.DefaultsAndOverrides.defaultSourceAddress;
        public Ice.EncodingVersion DefaultEncoding => Communicator.DefaultsAndOverrides.defaultEncoding;
        public int DefaultTimeout => Communicator.DefaultsAndOverrides.defaultTimeout;
        public int MessageSizeMax => Communicator.MessageSizeMax;
        public INetworkProxy? NetworkProxy => Communicator.NetworkProxy;

        public IEndpointFactory? GetEndpointFactory(short type) => Communicator.GetEndpointFactory(type);
        public void Resolve(string host, int port, Ice.EndpointSelectionType type, IPEndpoint endpt,
                            IEndpointConnectors callback) =>
            Communicator.Resolve(host, port, type, endpt, callback);
        public void setSndBufSizeWarn(short type, int size) => Communicator.SetSndBufSizeWarn(type, size);
        public void setRcvBufSizeWarn(short type, int size) => Communicator.SetRcvBufSizeWarn(type, size);

        internal Ice.BufSizeWarnInfo GetBufSizeWarn(short type) => Communicator.GetBufSizeWarn(type);
    }
}
