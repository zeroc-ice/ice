//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
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
        public override Transport Transport => Transport.TCP;

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
            timeout == Timeout ? this : CreateIPEndpoint(Host, Port, ConnectionId, HasCompressionFlag, timeout);

        public override IAcceptor GetAcceptor(string adapterName) =>
            new TcpAcceptor(this, Communicator, Host, Port, adapterName);
        public override ITransceiver? GetTransceiver() => null;

        internal TcpEndpoint(
            Communicator communicator,
            Protocol protocol,
            string host,
            int port,
            IPAddress? sourceAddress,
            int timeout,
            string connectionId,
            bool compressionFlag)
            : base(communicator, protocol, host, port, sourceAddress, connectionId)
        {
            Timeout = timeout;
            HasCompressionFlag = compressionFlag;
        }

        internal TcpEndpoint(InputStream istr, Protocol protocol)
            : base(istr, protocol)
        {
            Timeout = istr.ReadInt();
            HasCompressionFlag = istr.ReadBool();
        }

        internal TcpEndpoint(
            Communicator communicator,
            Protocol protocol,
            Dictionary<string, string?> options,
            bool oaEndpoint,
            string endpointString)
            : base(communicator, protocol, options, oaEndpoint, endpointString)
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
            new TcpConnector(this,
                             Communicator,
                             addr,
                             proxy,
                             SourceAddress,
                             Timeout,
                             ConnectionId);

        private protected override IPEndpoint CreateIPEndpoint(
            string host,
            int port,
            string connectionId,
            bool compressionFlag,
            int timeout) =>
            new TcpEndpoint(Communicator, Protocol, host, port, SourceAddress, timeout, connectionId, compressionFlag);

        internal virtual ITransceiver CreateTransceiver(
            string transport,
            StreamSocket socket,
            string? adapterName) =>
            new TcpTransceiver(transport, socket);
    }

    internal sealed class TcpEndpointFactory : IEndpointFactory
    {
        private Communicator _communicator;

        public Endpoint Create(
            Transport transport,
            Protocol protocol,
            Dictionary<string, string?> options,
            bool oaEndpoint,
            string endpointString)
        {
            Debug.Assert(transport == Transport.TCP); // we currently register this factory only for TCP
            return new TcpEndpoint(_communicator, protocol, options, oaEndpoint, endpointString);
        }

        public Endpoint Read(InputStream istr, Protocol protocol) => new TcpEndpoint(istr, protocol);

        internal TcpEndpointFactory(Communicator communicator) => _communicator = communicator;
    }
}
