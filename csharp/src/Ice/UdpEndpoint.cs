// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Net;
using System.Text;

namespace ZeroC.Ice
{
    /// <summary>The Endpoint class for the UDP transport.</summary>
    internal sealed class UdpEndpoint : IPEndpoint
    {
        public override bool IsDatagram => true;

        public override string? this[string option] =>
            option switch
            {
                "interface" => MulticastInterface,
                "ttl" => MulticastTtl.ToString(CultureInfo.InvariantCulture),
                "compress" => _hasCompressionFlag ? "true" : null,
                _ => base[option],
            };

        /// <summary>The local network interface used to send multicast datagrams.</summary>
        internal string? MulticastInterface { get; }

        /// <summary>The time-to-live of the multicast datagrams, in hops.</summary>
        internal int MulticastTtl { get; } = -1;

        private readonly bool _hasCompressionFlag;

        private int _hashCode;

        public override IAcceptor Acceptor(ObjectAdapter adapter) =>
            throw new InvalidOperationException();

        public override Connection CreateDatagramServerConnection(ObjectAdapter adapter)
        {
            Debug.Assert(Address != IPAddress.None); // i.e. not a DNS name

            var socket = new UdpSocket(this, Communicator);
            try
            {
                if (Communicator.TraceLevels.Transport >= 2)
                {
                    Communicator.Logger.Trace(TraceLevels.TransportCategory,
                        $"attempting to bind to {TransportName} socket\n{socket}");
                }
                Endpoint endpoint = socket.Bind(this);
                var multiStreamSocket = new Ice1NetworkSocket(socket, endpoint, adapter);
                return new UdpConnection(endpoint, multiStreamSocket, label: null, adapter);
            }
            catch (Exception)
            {
                socket.Dispose();
                throw;
            }
        }

        public override bool Equals(Endpoint? other)
        {
            if (ReferenceEquals(this, other))
            {
                return true;
            }
            return other is UdpEndpoint udpEndpoint &&
                _hasCompressionFlag == udpEndpoint._hasCompressionFlag &&
                MulticastInterface == udpEndpoint.MulticastInterface &&
                MulticastTtl == udpEndpoint.MulticastTtl &&
                base.Equals(udpEndpoint);
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
                hash.Add(_hasCompressionFlag);
                hash.Add(MulticastInterface);
                hash.Add(MulticastTtl);
                int hashCode = hash.ToHashCode();
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
            Debug.Assert(Protocol == Protocol.Ice1);

            base.AppendOptions(sb, optionSeparator);

            if (MulticastInterface != null)
            {
                Debug.Assert(MulticastInterface.Length > 0);
                bool addQuote = MulticastInterface.IndexOf(':') != -1;
                sb.Append(" --interface ");
                if (addQuote)
                {
                    sb.Append('"');
                }
                sb.Append(MulticastInterface);
                if (addQuote)
                {
                    sb.Append('"');
                }
            }

            if (MulticastTtl != -1)
            {
                sb.Append(" --ttl ");
                sb.Append(MulticastTtl.ToString(CultureInfo.InvariantCulture));
            }

            if (_hasCompressionFlag)
            {
                sb.Append(" -z");
            }
        }

        protected internal override Connection CreateConnection(
            bool secureOnly,
            IPEndPoint address,
            INetworkProxy? proxy,
            object? label)
        {
            Debug.Assert(secureOnly == false);
            UdpSocket socket = new(Communicator, address, SourceAddress, MulticastInterface, MulticastTtl);
            return new UdpConnection(this, new Ice1NetworkSocket(socket, this, adapter: null), label, adapter: null);
        }

        protected internal override void WriteOptions(OutputStream ostr)
        {
            Debug.Assert(Protocol == Protocol.Ice1);
            base.WriteOptions(ostr);
            ostr.WriteBool(_hasCompressionFlag);
        }

        internal static UdpEndpoint CreateIce1Endpoint(Transport transport, InputStream istr)
        {
            Debug.Assert(transport == Transport.UDP);
            return new UdpEndpoint(new EndpointData(transport,
                                                    host: istr.ReadString(),
                                                    port: ReadPort(istr),
                                                    Array.Empty<string>()),
                                   compress: istr.ReadBool(),
                                   istr.Communicator!);
        }

        internal static UdpEndpoint ParseIce1Endpoint(
            Transport transport,
            Dictionary<string, string?> options,
            Communicator communicator,
            bool oaEndpoint,
            string endpointString)
        {
            Debug.Assert(transport == Transport.UDP);

            (string host, ushort port) = ParseHostAndPort(options, oaEndpoint, endpointString);

            int ttl = -1;

            if (options.TryGetValue("--ttl", out string? argument))
            {
                if (argument == null)
                {
                    throw new FormatException($"no argument provided for --ttl option in endpoint `{endpointString}'");
                }
                try
                {
                    ttl = int.Parse(argument, CultureInfo.InvariantCulture);
                }
                catch (FormatException ex)
                {
                    throw new FormatException($"invalid TTL value `{argument}' in endpoint `{endpointString}'", ex);
                }

                if (ttl < 0)
                {
                    throw new FormatException($"TTL value `{argument}' out of range in endpoint `{endpointString}'");
                }
                options.Remove("--ttl");
            }

            string? multicastInterface = null;

            if (options.TryGetValue("--interface", out argument))
            {
                multicastInterface = argument ?? throw new FormatException(
                    $"no argument provided for --interface option in endpoint `{endpointString}'");

                if (multicastInterface == "*")
                {
                    if (oaEndpoint)
                    {
                        multicastInterface = null;
                    }
                    else
                    {
                        throw new FormatException($"`--interface *' not valid for proxy endpoint `{endpointString}'");
                    }
                }
                options.Remove("--interface");
            }

            return new UdpEndpoint(new EndpointData(transport, host, port, Array.Empty<string>()),
                                   ParseCompress(options, endpointString),
                                   ttl,
                                   multicastInterface,
                                   options,
                                   communicator,
                                   oaEndpoint,
                                   endpointString);
        }

        // Constructor for ice1 unmarshaling
        private UdpEndpoint(EndpointData data, bool compress, Communicator communicator)
            : base(data, communicator, Protocol.Ice1) =>
            _hasCompressionFlag = compress;

        // Constructor for ice1 parsing
        private UdpEndpoint(
            EndpointData data,
            bool compress,
            int ttl,
            string? multicastInterface,
            Dictionary<string, string?> options,
            Communicator communicator,
            bool oaEndpoint,
            string endpointString)
            : base(data, options, communicator, oaEndpoint, endpointString)
        {
            _hasCompressionFlag = compress;
            MulticastTtl = ttl;
            MulticastInterface = multicastInterface;
        }

        // Clone constructor
        private UdpEndpoint(UdpEndpoint endpoint, string host, ushort port)
            : base(endpoint, host, port)
        {
            MulticastInterface = endpoint.MulticastInterface;
            MulticastTtl = endpoint.MulticastTtl;
            _hasCompressionFlag = endpoint._hasCompressionFlag;
        }
        private protected override IPEndpoint Clone(string host, ushort port) =>
            new UdpEndpoint(this, host, port);
    }
}
