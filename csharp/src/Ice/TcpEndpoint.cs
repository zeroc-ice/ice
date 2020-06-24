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
        public override bool HasCompressionFlag { get; }
        public override bool IsDatagram => false;
        public override bool IsSecure => Transport == Transport.SSL;
        public override int Timeout { get; }
        public override Transport Transport { get; }

        private int _hashCode = 0;

        public override bool Equals(Endpoint? other)
        {
            if (ReferenceEquals(this, other))
            {
                return true;
            }

            if (other is TcpEndpoint tcpEndpoint)
            {
                if (HasCompressionFlag != tcpEndpoint.HasCompressionFlag)
                {
                    return false;
                }

                if (Timeout != tcpEndpoint.Timeout)
                {
                    return false;
                }

                if (Transport != tcpEndpoint.Transport)
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
                hash.Add(Transport);
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
            Transport transport,
            string host,
            int port,
            IPAddress? sourceAddress,
            int timeout,
            string connectionId,
            bool compressionFlag)
            : base(communicator, protocol, host, port, sourceAddress, connectionId)
        {
            Transport = transport;
            HasCompressionFlag = compressionFlag;
            Timeout = timeout;
        }

        internal TcpEndpoint(InputStream istr, Protocol protocol, Transport transport)
            : base(istr, protocol)
        {
            Transport = transport;
            Timeout = istr.ReadInt();
            HasCompressionFlag = istr.ReadBool();
        }

        internal TcpEndpoint(
            Communicator communicator,
            Protocol protocol,
            Transport transport,
            Dictionary<string, string?> options,
            bool oaEndpoint,
            string endpointString)
            : base(communicator, protocol, options, oaEndpoint, endpointString)
        {
            Transport = transport;
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
            new TcpEndpoint(Communicator,
                            Protocol,
                            Transport,
                            host,
                            port,
                            SourceAddress,
                            timeout,
                            connectionId,
                            compressionFlag);

        internal virtual ITransceiver CreateTransceiver(
            string transport,
            StreamSocket socket,
            string? adapterName)
        {
            if (transport == "ssl" || transport == "wss")
            {
                return new SslTransceiver(Communicator,
                                          new TcpTransceiver(transport, socket),
                                          adapterName ?? Host,
                                          adapterName != null);
            }
            else
            {
                return new TcpTransceiver(transport, socket);
            }
        }
    }

    internal sealed class TcpEndpointFactory : IEndpointFactory
    {
        private readonly Communicator _communicator;

        public Endpoint Create(
            Transport transport,
            Protocol protocol,
            Dictionary<string, string?> options,
            bool oaEndpoint,
            string endpointString)
        {
            Debug.Assert(transport == Transport.TCP || transport == Transport.SSL);
            return new TcpEndpoint(_communicator, protocol, transport, options, oaEndpoint, endpointString);
        }

        public Endpoint Read(InputStream istr, Protocol protocol, Transport transport)
        {
            Debug.Assert(transport == Transport.TCP || transport == Transport.SSL);
            return new TcpEndpoint(istr, protocol, transport);
        }

        internal TcpEndpointFactory(Communicator communicator) => _communicator = communicator;
    }
}
