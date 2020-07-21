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
        // We cannot move HasCompressionFlag to IPEndpoint because of it's marshaled after the timeout in TcpEndpoint
        public override bool HasCompressionFlag { get; }
        public override bool IsDatagram => true;

        public override string? this[string option] =>
            option switch
            {
                "interface" => McastInterface,
                "ttl" => McastTtl.ToString(CultureInfo.InvariantCulture),
                _ => base[option],
            };

        public override Transport Transport => Transport.UDP;

        /// <summary>The local network interface used to send multicast datagrams.</summary>
        internal string McastInterface { get; } = "";

        /// <summary>The time-to-live of the multicast datagrams, in hops.</summary>
        internal int McastTtl { get; } = -1;

        private readonly bool _connect;
        private int _hashCode = 0;

        public override bool Equals(Endpoint? other)
        {
            if (ReferenceEquals(this, other))
            {
                return true;
            }
            return other is UdpEndpoint udpEndpoint &&
                HasCompressionFlag == udpEndpoint.HasCompressionFlag &&
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

        public override string OptionsToString()
        {
            var sb = new StringBuilder(base.OptionsToString());

            if (McastInterface.Length > 0)
            {
                bool addQuote = McastInterface.IndexOf(':') != -1;
                sb.Append(" --interface ");
                if (addQuote)
                {
                    sb.Append("\"");
                }
                sb.Append(McastInterface);
                if (addQuote)
                {
                    sb.Append("\"");
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
            return sb.ToString();
        }

        public override void IceWritePayload(OutputStream ostr)
        {
            Debug.Assert(Protocol == Protocol.Ice1);
            base.IceWritePayload(ostr);
            ostr.WriteBool(HasCompressionFlag);
        }

        public override Endpoint NewTimeout(TimeSpan timeout) => this;

        public override IAcceptor? GetAcceptor(string adapterName) => null;

        public override ITransceiver GetTransceiver() =>
            new UdpTransceiver(this, Communicator, Host, Port, McastInterface, _connect);

        internal UdpEndpoint(
            Communicator communicator,
            Protocol protocol,
            string host,
            ushort port,
            IPAddress? sourceAddress,
            string mcastInterface,
            int mttl,
            bool connect,
            bool compressionFlag)
            : base(communicator, protocol, host, port, sourceAddress)
        {
            McastInterface = mcastInterface;
            McastTtl = mttl;
            _connect = connect;
            HasCompressionFlag = compressionFlag;
        }

        // Constructor for unmarshaling.
        // TODO: should be ice1-only
        internal UdpEndpoint(InputStream istr, Communicator communicator, Protocol protocol)
            : base(istr, communicator, protocol)
        {
            _connect = false;
            if (protocol == Protocol.Ice1)
            {
                HasCompressionFlag = istr.ReadBool();
            }
            else
            {
                SkipUnknownOptions(istr, istr.ReadSize());
            }
        }

        // Constructor for URI parsing
        internal UdpEndpoint(
            Communicator communicator,
            Protocol protocol,           // TODO: temporary, should always be ice1
            string host,
            ushort port,
            Dictionary<string, string> options,
            bool oaEndpoint,
            string endpointString)
            : base(communicator, protocol, host, port, options, oaEndpoint)
        {
            _connect = false; // TODO: what is this connect about?
            string? value;

            if (protocol == Protocol.Ice1)
            {
                if (options.TryGetValue("compress", out value))
                {
                    if (value == "true")
                    {
                        HasCompressionFlag = true;
                    }
                    else if (value != "false")
                    {
                        throw new FormatException(
                            $"invalid compress value `{value}' in endpoint `{endpointString}'");
                    }
                    options.Remove("compress");
                }
            }

            // Parse local options
            if (options.TryGetValue("ttl", out value))
            {
                try
                {
                    McastTtl = int.Parse(value, CultureInfo.InvariantCulture);
                }
                catch (FormatException ex)
                {
                    throw new FormatException($"invalid ttl value `{value}' in endpoint `{endpointString}'", ex);
                }

                if (McastTtl < 0)
                {
                    throw new FormatException($"ttl value `{value}' out of range in endpoint `{endpointString}'");
                }
                options.Remove("ttl");
            }

            if (options.TryGetValue("interface", out value))
            {
                // interface is percent escaped, for example "%lo" in the interface string is encoded as "%25lo".
                McastInterface = Uri.UnescapeDataString(value);
                if (McastInterface == "*")
                {
                    if (oaEndpoint)
                    {
                        McastInterface = "";
                    }
                    else
                    {
                        throw new FormatException(
                            $"`interface=*' is not valid for proxy endpoint `{endpointString}'");
                    }
                }
                options.Remove("interface");
            }
        }

        internal UdpEndpoint(
            Communicator communicator,
            Protocol protocol,
            Dictionary<string, string?> options,
            bool oaEndpoint,
            string endpointString)
            : base(communicator, protocol, options, oaEndpoint, endpointString)
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

        private protected override IPEndpoint CreateIPEndpoint(
            string host,
            ushort port,
            bool compressionFlag,
            TimeSpan timeout) =>
            new UdpEndpoint(Communicator,
                            Protocol,
                            host,
                            port,
                            SourceAddress,
                            McastInterface,
                            McastTtl,
                            _connect,
                            compressionFlag);
    }
}
