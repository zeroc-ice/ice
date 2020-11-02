// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace ZeroC.Ice
{
    /// <summary>The Endpoint class for the TCP transport.</summary>
    internal class TcpEndpoint : IPEndpoint
    {
        public override bool IsDatagram => false;
        public override bool IsSecure => Transport == Transport.SSL;

        public override string? this[string option] =>
            option switch
            {
                "compress" => HasCompressionFlag ? "true" : null,
                "timeout" => Timeout != DefaultTimeout ?
                             Timeout.TotalMilliseconds.ToString(CultureInfo.InvariantCulture) : null,
                _ => base[option],
            };

        private protected bool HasCompressionFlag { get; }
        private protected TimeSpan Timeout { get; } = DefaultTimeout;

        /// <summary>The default timeout for ice1 endpoints.</summary>
        protected static readonly TimeSpan DefaultTimeout = TimeSpan.FromSeconds(60);

        private int _hashCode;

        public override IAcceptor Acceptor(IConnectionManager manager, ObjectAdapter adapter) =>
            new TcpAcceptor(this, manager, adapter);

        public override Connection CreateDatagramServerConnection(ObjectAdapter adapter) =>
            throw new InvalidOperationException();

        public override bool Equals(Endpoint? other)
        {
            if (ReferenceEquals(this, other))
            {
                return true;
            }

            if (Protocol == Protocol.Ice1)
            {
                return other is TcpEndpoint tcpEndpoint &&
                    Timeout == tcpEndpoint.Timeout &&
                    base.Equals(tcpEndpoint);
            }
            else
            {
                return base.Equals(other);
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
                int hashCode;
                if (Protocol == Protocol.Ice1)
                {
                    hashCode = HashCode.Combine(base.GetHashCode(), Timeout);
                }
                else
                {
                    hashCode = base.GetHashCode();
                }

                if (hashCode == 0) // 0 is not a valid value as it means "not initialized".
                {
                    hashCode = 1;
                }
                _hashCode = hashCode;
                return _hashCode;
            }
        }

        protected internal override void AppendOptions(StringBuilder sb, char optionSeparator)
        {
            base.AppendOptions(sb, optionSeparator);
            if (Protocol == Protocol.Ice1)
            {
                // InfiniteTimeSpan yields -1 and we use -1 instead of "infinite" for compatibility with Ice 3.5.
                sb.Append(" -t ");
                sb.Append(Timeout.TotalMilliseconds);

                if (HasCompressionFlag)
                {
                    sb.Append(" -z");
                }
            }
        }

        protected internal override void WriteOptions(OutputStream ostr)
        {
            Debug.Assert(Protocol == Protocol.Ice1);
            base.WriteOptions(ostr);
            ostr.WriteInt((int)Timeout.TotalMilliseconds);
            ostr.WriteBool(HasCompressionFlag);
        }

        internal static TcpEndpoint CreateIce1Endpoint(Transport transport, InputStream istr)
        {
            Debug.Assert(transport == Transport.TCP || transport == Transport.SSL);

            // This is correct in C# since arguments are evaluated left-to-right. This would not be correct in C++ where
            // the order of evaluation of function arguments is undefined.
            return new TcpEndpoint(new EndpointData(transport,
                                                    host: istr.ReadString(),
                                                    port: ReadPort(istr),
                                                    Array.Empty<string>()),
                                   timeout: TimeSpan.FromMilliseconds(istr.ReadInt()),
                                   compress: istr.ReadBool(),
                                   istr.Communicator!);
        }

        internal static TcpEndpoint CreateIce2Endpoint(EndpointData data, Communicator communicator)
        {
            Debug.Assert(data.Transport == Transport.TCP || data.Transport == Transport.SSL); // TODO: remove SSL

            // Drop all options since we don't understand any.
            return new TcpEndpoint(new EndpointData(data.Transport, data.Host, data.Port, Array.Empty<string>()),
                                   communicator);
        }

        internal static TcpEndpoint ParseIce1Endpoint(
            Transport transport,
            Dictionary<string, string?> options,
            Communicator communicator,
            bool oaEndpoint,
            string endpointString)
        {
            Debug.Assert(transport == Transport.TCP || transport == Transport.SSL);
            (string host, ushort port) = ParseHostAndPort(options, oaEndpoint, endpointString);
            return new TcpEndpoint(new EndpointData(transport, host, port, Array.Empty<string>()),
                                   ParseTimeout(options, endpointString),
                                   ParseCompress(options, endpointString),
                                   options,
                                   communicator,
                                   oaEndpoint,
                                   endpointString);
        }

        internal static TcpEndpoint ParseIce2Endpoint(
            Transport transport,
            string host,
            ushort port,
            Dictionary<string, string> options,
            Communicator communicator,
            bool oaEndpoint)
        {
            Debug.Assert(transport == Transport.TCP || transport == Transport.SSL);
            return new TcpEndpoint(new EndpointData(transport, host, port, Array.Empty<string>()),
                                   options,
                                   communicator,
                                   oaEndpoint);
        }

        internal virtual Connection CreateConnection(
            IConnectionManager manager,
            MultiStreamTransceiverWithUnderlyingTransceiver transceiver,
            IConnector? connector,
            string connectionId,
            ObjectAdapter? adapter) =>
            new TcpConnection(manager, this, transceiver, connector, connectionId, adapter);

        private protected static TimeSpan ParseTimeout(Dictionary<string, string?> options, string endpointString)
        {
            TimeSpan timeout = DefaultTimeout;

            if (options.TryGetValue("-t", out string? argument))
            {
                if (argument == null)
                {
                    throw new FormatException($"no argument provided for -t option in endpoint `{endpointString}'");
                }
                if (argument == "infinite")
                {
                    timeout = System.Threading.Timeout.InfiniteTimeSpan;
                }
                else
                {
                    try
                    {
                        timeout = TimeSpan.FromMilliseconds(int.Parse(argument, CultureInfo.InvariantCulture));
                    }
                    catch (FormatException ex)
                    {
                        throw new FormatException(
                            $"invalid timeout value `{argument}' in endpoint `{endpointString}'",
                            ex);
                    }
                    if (timeout <= TimeSpan.Zero)
                    {
                        throw new FormatException($"invalid timeout value `{argument}' in endpoint `{endpointString}'");
                    }
                }
                options.Remove("-t");
            }
            return timeout;
        }

        // Constructor for ice1 unmarshaling.
        private protected TcpEndpoint(EndpointData data, TimeSpan timeout, bool compress, Communicator communicator)
            : base(data, communicator, Protocol.Ice1)
        {
            Timeout = timeout;
            HasCompressionFlag = compress;
        }

        // Constructor for ice2 unmarshaling.
        private protected TcpEndpoint(EndpointData data, Communicator communicator)
            : base(data, communicator, Protocol.Ice2)
        {
        }

        // Constructor for ice1 parsing.
        private protected TcpEndpoint(
            EndpointData data,
            TimeSpan timeout,
            bool compress,
            Dictionary<string, string?> options,
            Communicator communicator,
            bool oaEndpoint,
            string endpointString)
            : base(data, options, communicator, oaEndpoint, endpointString)
        {
            Timeout = timeout;
            HasCompressionFlag = compress;
        }

        // Constructor for ice2 parsing.
        private protected TcpEndpoint(
            EndpointData data,
            Dictionary<string, string> options,
            Communicator communicator,
            bool oaEndpoint)
            : base(data, options, communicator, oaEndpoint)
        {
        }

        // Clone constructor
        private protected TcpEndpoint(TcpEndpoint endpoint, string host, ushort port)
            : base(endpoint, host, port)
        {
            HasCompressionFlag = endpoint.HasCompressionFlag;
            Timeout = endpoint.Timeout;
        }

        private protected override IPEndpoint Clone(string host, ushort port) =>
            new TcpEndpoint(this, host, port);

        private protected override IConnector CreateConnector(EndPoint addr, INetworkProxy? proxy) =>
            new TcpConnector(this, addr, proxy);

        internal virtual ITransceiver CreateTransceiver(IConnector connector, EndPoint addr, INetworkProxy? proxy)
        {
            ITransceiver transceiver = new TcpTransceiver(Communicator, connector, addr, proxy, SourceAddress);
            if (IsSecure)
            {
                transceiver = new SslTransceiver(Communicator, transceiver, Host, false, connector);
            }
            return transceiver;
        }

        internal virtual ITransceiver CreateTransceiver(Socket socket, string adapterName)
        {
            ITransceiver transceiver = new TcpTransceiver(Communicator, socket);
            if (IsSecure)
            {
                transceiver = new SslTransceiver(Communicator, transceiver, adapterName, true);
            }
            return transceiver;
        }
    }
}
