//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Net;
using System.Text;

namespace ZeroC.Ice
{
    /// <summary>The Endpoint class for the TCP transport.</summary>
    public sealed class TcpEndpoint : IPEndpoint
    {
        public override bool IsDatagram => false;
        public override bool HasCompressionFlag { get; }

        public override int Timeout { get; }

        private int _hashCode = 0;

        public override bool Equals(Endpoint? other)
        {
            if (ReferenceEquals(this, other))
            {
                return true;
            }
            if (other is TcpEndpoint tcpEndpoint)
            {
                if (Timeout != tcpEndpoint.Timeout)
                {
                    return false;
                }
                if (HasCompressionFlag != tcpEndpoint.HasCompressionFlag)
                {
                    return false;
                }
                return base.Equals(tcpEndpoint);
            }
            else
            {
                return false;
            }
        }

        public override int GetHashCode()
        {
            // This code is thread safe because reading/writing _hashCode (an int) is atomic.
            if (_hashCode != 0)
            {
                // Return cached value
                return _hashCode;
            }
            else
            {
                var hash = new HashCode();
                hash.Add(base.GetHashCode());
                hash.Add(HasCompressionFlag);
                hash.Add(Timeout);
                int hashCode = hash.ToHashCode();
                if (hashCode == 0) // 0 is not a valid value as it means "not initialized".
                {
                    hashCode = 1;
                }
                _hashCode = hashCode;
                return _hashCode;
            }
        }

        public override string OptionsToString()
        {
            var sb = new StringBuilder(base.OptionsToString());

            if (Timeout == -1)
            {
                sb.Append(" -t infinite");
            }
            else
            {
                sb.Append(" -t ");
                sb.Append(Timeout.ToString(CultureInfo.InvariantCulture));
            }

            if (HasCompressionFlag)
            {
                sb.Append(" -z");
            }
            return sb.ToString();
        }

        public override void IceWritePayload(OutputStream ostr)
        {
            base.IceWritePayload(ostr);
            ostr.WriteInt(Timeout);
            ostr.WriteBool(HasCompressionFlag);
        }

        public override Endpoint NewTimeout(int timeout) =>
            timeout == Timeout ? this :
                new TcpEndpoint(Instance, Host, Port, SourceAddress, timeout, ConnectionId, HasCompressionFlag);

        public override Endpoint NewCompressionFlag(bool compressionFlag) =>
            compressionFlag == HasCompressionFlag ? this :
                new TcpEndpoint(Instance, Host, Port, SourceAddress, Timeout, ConnectionId, compressionFlag);

        public override IAcceptor GetAcceptor(string adapterName) => new TcpAcceptor(this, Instance, Host, Port);
        public override ITransceiver? GetTransceiver() => null;

        internal TcpEndpoint GetEndpoint(TcpAcceptor acceptor)
        {
            int port = acceptor.EffectivePort();
            if (port == Port)
            {
                return this;
            }
            else
            {
                return new TcpEndpoint(Instance, Host, port, SourceAddress, Timeout, ConnectionId, HasCompressionFlag);
            }
        }

        internal TcpEndpoint(TransportInstance instance, string host, int port, IPAddress? sourceAddress,
                             int timeout, string connectionId, bool compressionFlag) :
            base(instance, host, port, sourceAddress, connectionId)
        {
            Timeout = timeout;
            HasCompressionFlag = compressionFlag;
        }

        internal TcpEndpoint(TransportInstance instance, InputStream s) :
            base(instance, s)
        {
            Timeout = s.ReadInt();
            HasCompressionFlag = s.ReadBool();
        }

        internal TcpEndpoint(TransportInstance instance, string endpointString, Dictionary<string, string?> options,
                             bool oaEndpoint)
            : base(instance, endpointString, options, oaEndpoint)
        {
            if (options.TryGetValue("-t", out string? argument))
            {
                if (argument == null)
                {
                    throw new FormatException($"no argument provided for -t option in endpoint `{endpointString}'");
                }
                if (argument == "infinite")
                {
                    Timeout = -1;
                }
                else
                {
                    try
                    {
                        Timeout = int.Parse(argument, CultureInfo.InvariantCulture);
                    }
                    catch (FormatException ex)
                    {
                        throw new FormatException($"invalid timeout value `{argument}' in endpoint `{endpointString}'",
                             ex);
                    }
                    if (Timeout < 1)
                    {
                        throw new FormatException($"invalid timeout value `{argument}' in endpoint `{endpointString}'");
                    }
                }
                options.Remove("-t");
            }
            else
            {
                Timeout = Instance.DefaultTimeout;
            }

            if (options.TryGetValue("-z", out argument))
            {
                if (argument != null)
                {
                    throw new FormatException(
                        $"unexpected argument `{argument}' provided for -z option in `{endpointString}'");
                }
                HasCompressionFlag = true;
                options.Remove("-z");
            }
        }

        private protected override IConnector CreateConnector(EndPoint addr, INetworkProxy? proxy) =>
            new TcpConnector(Instance, addr, proxy, SourceAddress, Timeout, ConnectionId);

        private protected override IPEndpoint CreateEndpoint(string host, int port, string connectionId) =>
            new TcpEndpoint(Instance, host, port, SourceAddress, Timeout, connectionId, HasCompressionFlag);
    }

    internal sealed class TcpEndpointFactory : IEndpointFactory
    {
        private readonly TransportInstance _instance;

        public void Initialize()
        {
        }
        public void Destroy()
        {
        }

        public EndpointType Type() => _instance.Type;
        public string Transport() => _instance.Transport;

        public Endpoint Create(string endpointString, Dictionary<string, string?> options, bool oaEndpoint) =>
            new TcpEndpoint(_instance, endpointString, options, oaEndpoint);

        public Endpoint Read(InputStream s) => new TcpEndpoint(_instance, s);
        public IEndpointFactory Clone(TransportInstance instance) => new TcpEndpointFactory(instance);

        internal TcpEndpointFactory(TransportInstance instance) => _instance = instance;
    }
}
