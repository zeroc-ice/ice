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
    /// <summary>The Endpoint class for the UDP transport.</summary>
    internal sealed class UdpEndpoint : IPEndpoint
    {
        public override bool IsDatagram => true;

        public override string? this[string option] =>
            option switch
            {
                "interface" => McastInterface,
                "ttl" => McastTtl.ToString(CultureInfo.InvariantCulture),
                "compress" => HasCompressionFlag ? "true" : null,
                _ => base[option],
            };

        public override Transport Transport => Transport.UDP;

        /// <summary>The local network interface used to send multicast datagrams.</summary>
        internal string McastInterface { get; } = "";

        /// <summary>The time-to-live of the multicast datagrams, in hops.</summary>
        internal int McastTtl { get; } = -1;

        private bool HasCompressionFlag { get; }

        private readonly bool _connect;
        private int _hashCode;

        public override bool Equals(Endpoint? other)
        {
            if (ReferenceEquals(this, other))
            {
                return true;
            }
            return other is UdpEndpoint udpEndpoint &&
                _connect == udpEndpoint._connect &&
                McastInterface == udpEndpoint.McastInterface &&
                McastTtl == udpEndpoint.McastTtl &&
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
                hash.Add(_connect);
                hash.Add(HasCompressionFlag);
                hash.Add(McastInterface);
                hash.Add(McastTtl);
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

            if (McastInterface.Length > 0)
            {
                bool addQuote = McastInterface.IndexOf(':') != -1;
                sb.Append(" --interface ");
                if (addQuote)
                {
                    sb.Append('"');
                }
                sb.Append(McastInterface);
                if (addQuote)
                {
                    sb.Append('"');
                }
            }

            if (McastTtl != -1)
            {
                sb.Append(" --ttl ");
                sb.Append(McastTtl.ToString(CultureInfo.InvariantCulture));
            }

            if (_connect)
            {
                sb.Append(" -c");
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

        public override Connection CreateConnection(
             IConnectionManager manager,
             ITransceiver? transceiver,
             IConnector? connector,
             string connectionId,
             ObjectAdapter? adapter) => new UdpConnection(manager,
                                                          this,
                                                          new Ice1BinaryConnection(transceiver!, this, adapter),
                                                          connector,
                                                          connectionId,
                                                          adapter);

        public override ITransceiver GetTransceiver() =>
            new UdpTransceiver(this, Communicator, Host, Port, McastInterface, _connect);

        // Constructor for unmarshaling.
        internal UdpEndpoint(InputStream istr, Communicator communicator)
            : base(istr, communicator, Protocol.Ice1)
        {
           _connect = false;
           HasCompressionFlag = istr.ReadBool();
        }

        // Constructor for ice1 endpoint parsing.
        internal UdpEndpoint(
            Communicator communicator,
            Dictionary<string, string?> options,
            bool oaEndpoint,
            string endpointString)
            : base(communicator, options, oaEndpoint, endpointString)
        {
            if (options.TryGetValue("-c", out string? argument))
            {
                if (argument != null)
                {
                    throw new FormatException(
                        $"unexpected argument `{argument}' provided for -c option in `{endpointString}'");
                }
                _connect = true;
                options.Remove("-c");
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

            if (options.TryGetValue("--ttl", out argument))
            {
                if (argument == null)
                {
                    throw new FormatException($"no argument provided for --ttl option in endpoint `{endpointString}'");
                }
                try
                {
                    McastTtl = int.Parse(argument, CultureInfo.InvariantCulture);
                }
                catch (FormatException ex)
                {
                    throw new FormatException($"invalid TTL value `{argument}' in endpoint `{endpointString}'", ex);
                }

                if (McastTtl < 0)
                {
                    throw new FormatException($"TTL value `{argument}' out of range in endpoint `{endpointString}'");
                }
                options.Remove("--ttl");
            }

            if (options.TryGetValue("--interface", out argument))
            {
                McastInterface = argument ?? throw new FormatException(
                    $"no argument provided for --interface option in endpoint `{endpointString}'");

                if (McastInterface == "*")
                {
                    if (oaEndpoint)
                    {
                        McastInterface = "";
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
            McastInterface = endpoint.McastInterface;
            McastTtl = endpoint.McastTtl;
            _connect = endpoint._connect;
            HasCompressionFlag = endpoint.HasCompressionFlag;
        }

        private protected override IPEndpoint Clone(string host, ushort port) =>
            new UdpEndpoint(this, host, port);
    }
}
