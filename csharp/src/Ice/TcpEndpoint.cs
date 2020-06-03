//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Net;
using System.Text;

namespace ZeroC.Ice
{
    /// <summary>The Endpoint class for the TCP transport.</summary>
    public class TcpEndpoint : IPEndpoint
    {
        public override bool IsDatagram => false;
        public override bool HasCompressionFlag { get; }
        public override int Timeout { get; }
        public override EndpointType Type => EndpointType.TCP;

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
            timeout == Timeout ? this : CreateEndpoint(Host, Port, ConnectionId, HasCompressionFlag, timeout);

        public override IAcceptor GetAcceptor(string adapterName) =>
            new TcpAcceptor(this, Communicator, Name, Host, Port, adapterName);
        public override ITransceiver? GetTransceiver() => null;

        internal TcpEndpoint(Communicator communicator, string host, int port, IPAddress? sourceAddress, int timeout,
            string connectionId, bool compressionFlag) :
            base(communicator, host, port, sourceAddress, connectionId)
        {
            Timeout = timeout;
            HasCompressionFlag = compressionFlag;
        }

        internal TcpEndpoint(Communicator communicator, InputStream istr) : base(communicator, istr)
        {
            Timeout = istr.ReadInt();
            HasCompressionFlag = istr.ReadBool();
        }

        internal TcpEndpoint(Communicator communicator, string endpointString, Dictionary<string, string?> options,
            bool oaEndpoint) : base(communicator, endpointString, options, oaEndpoint)
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
                Timeout = Communicator.DefaultTimeout;
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
            new TcpConnector(this, Communicator, Name, Type, addr, proxy, SourceAddress, Timeout, ConnectionId);
        private protected override IPEndpoint CreateEndpoint(string host, int port, string connectionId,
            bool compressionFlag, int timeout) =>
            new TcpEndpoint(Communicator, host, port, SourceAddress, timeout, connectionId, compressionFlag);

        internal virtual ITransceiver CreateTransceiver(
            string transport,
            StreamSocket socket,
            string? adapterName) =>
            new TcpTransceiver(transport, socket);
    }

    internal sealed class TcpEndpointFactory : IEndpointFactory
    {
        public EndpointType Type => EndpointType.TCP;
        public string Name => "tcp";

        private Communicator Communicator { get; }

        public Endpoint Create(string endpointString, Dictionary<string, string?> options, bool oaEndpoint) =>
            new TcpEndpoint(Communicator, endpointString, options, oaEndpoint);

        public Endpoint Read(InputStream istr) => new TcpEndpoint(Communicator, istr);

        internal TcpEndpointFactory(Communicator communicator) => Communicator = communicator;
    }
}
