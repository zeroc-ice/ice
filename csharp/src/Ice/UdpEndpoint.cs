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
                "compress" => HasCompressionFlag ? "true" : null,
                _ => base[option],
            };

        public override Transport Transport => Transport.UDP;

        /// <summary>The local network interface used to send multicast datagrams.</summary>
        internal string MulticastInterface { get; } = "";

        /// <summary>The time-to-live of the multicast datagrams, in hops.</summary>
        internal int MulticastTtl { get; } = -1;

        private bool HasCompressionFlag { get; }

        private int _hashCode;

        public override IAcceptor Acceptor(IConnectionManager manager, ObjectAdapter adapter) =>
            throw new InvalidOperationException();

        public override bool Equals(Endpoint? other)
        {
            if (ReferenceEquals(this, other))
            {
                return true;
            }
            return other is UdpEndpoint udpEndpoint &&
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
                hash.Add(HasCompressionFlag);
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

            if (MulticastInterface.Length > 0)
            {
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

            if (HasCompressionFlag)
            {
                sb.Append(" -z");
            }
        }

        protected internal override void WriteOptions(OutputStream ostr)
        {
            // TODO: temporary, should be ice1-only
            if (Protocol == Protocol.Ice1)
            {
                base.WriteOptions(ostr);
                ostr.WriteBool(HasCompressionFlag);
            }
            else
            {
                ostr.WriteSize(0);
            }
        }

        public override Connection CreateDatagramServerConnection(ObjectAdapter adapter)
        {
            var transceiver = new UdpTransceiver(this, Communicator);
            try
            {
                if (Communicator.TraceLevels.Transport >= 2)
                {
                    Communicator.Logger.Trace(Communicator.TraceLevels.TransportCategory,
                        $"attempting to bind to {TransportName} socket\n{transceiver}");
                }
                Endpoint endpoint = transceiver.Bind(this);
                var multiStreamTransceiver = new LegacyTransceiver(transceiver, endpoint, adapter);
                return new UdpConnection(null, endpoint, multiStreamTransceiver, null, "", adapter);
            }
            catch (Exception)
            {
                transceiver.Dispose();
                throw;
            }
        }

        // Constructor for unmarshaling.
        internal UdpEndpoint(InputStream istr)
            : base(istr, Protocol.Ice1) => HasCompressionFlag = istr.ReadBool();

        // Constructor for ice1 endpoint parsing.
        internal UdpEndpoint(
            Communicator communicator,
            Dictionary<string, string?> options,
            bool oaEndpoint,
            string endpointString)
            : base(communicator, options, oaEndpoint, endpointString)
        {
            if (options.TryGetValue("-z", out string? argument))
            {
                if (argument != null)
                {
                    throw new FormatException(
                        $"unexpected argument `{argument}' provided for -z option in `{endpointString}'");
                }
                HasCompressionFlag = true;
                options.Remove("-z");
            }

            if (options.TryGetValue("--ttl", out argument))
            {
                if (argument == null)
                {
                    throw new FormatException($"no argument provided for --ttl option in endpoint `{endpointString}'");
                }
                try
                {
                    MulticastTtl = int.Parse(argument, CultureInfo.InvariantCulture);
                }
                catch (FormatException ex)
                {
                    throw new FormatException($"invalid TTL value `{argument}' in endpoint `{endpointString}'", ex);
                }

                if (MulticastTtl < 0)
                {
                    throw new FormatException($"TTL value `{argument}' out of range in endpoint `{endpointString}'");
                }
                options.Remove("--ttl");
            }

            if (options.TryGetValue("--interface", out argument))
            {
                MulticastInterface = argument ?? throw new FormatException(
                    $"no argument provided for --interface option in endpoint `{endpointString}'");

                if (MulticastInterface == "*")
                {
                    if (oaEndpoint)
                    {
                        MulticastInterface = "";
                    }
                    else
                    {
                        throw new FormatException($"`--interface *' not valid for proxy endpoint `{endpointString}'");
                    }
                }
                options.Remove("--interface");
            }
        }

        private protected override IConnector CreateConnector(EndPoint addr, INetworkProxy? _) =>
            new UdpConnector(this, addr);

        // Clone constructor
        private UdpEndpoint(UdpEndpoint endpoint, string host, ushort port)
            : base(endpoint, host, port)
        {
            MulticastInterface = endpoint.MulticastInterface;
            MulticastTtl = endpoint.MulticastTtl;
            HasCompressionFlag = endpoint.HasCompressionFlag;
        }

        private protected override IPEndpoint Clone(string host, ushort port) =>
            new UdpEndpoint(this, host, port);
    }
}
