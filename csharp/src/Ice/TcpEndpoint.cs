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
    internal class TcpEndpoint : IPEndpoint
    {
        public override bool IsDatagram => false;
        public override bool IsSecure => Transport == Transport.SSL;
        public override Transport Transport { get; }

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
                if (Timeout == System.Threading.Timeout.InfiniteTimeSpan)
                {
                    // We use "-1" instead of "infinite" for compatibility with Ice 3.5.
                    sb.Append(" -t -1");
                }
                else
                {
                    sb.Append(" -t ");
                    sb.Append(Timeout.TotalMilliseconds);
                }

                if (HasCompressionFlag)
                {
                    sb.Append(" -z");
                }
            }
        }

        protected internal override void WriteOptions(OutputStream ostr)
        {
            if (Protocol == Protocol.Ice1)
            {
                base.WriteOptions(ostr);
                ostr.WriteInt((int)Timeout.TotalMilliseconds);
                ostr.WriteBool(HasCompressionFlag);
            }
            else
            {
                ostr.WriteSize(0); // empty sequence of options
            }
        }

        public override Connection CreateConnection(
             IConnectionManager manager,
             ITransceiver? transceiver,
             IConnector? connector,
             string connectionId,
             ObjectAdapter? adapter) =>
             new TcpConnection(manager,
                               this,
                               Protocol == Protocol.Ice1 ? (IBinaryConnection)
                                 new Ice1BinaryConnection(transceiver!, this, adapter) :
                                 new SlicBinaryConnection(transceiver!, this, adapter),
                               connector,
                               connectionId,
                               adapter);
        public override ITransceiver? GetTransceiver() => null;

        internal TcpEndpoint(
            Communicator communicator,
            Transport transport,
            Protocol protocol,
            string host,
            ushort port,
            IPAddress? sourceAddress,
            TimeSpan timeout,
            bool compressionFlag)
            : base(communicator, protocol, host, port, sourceAddress)
        {
            Transport = transport;
            HasCompressionFlag = compressionFlag;
            Timeout = timeout;
        }

        // Constructor for unmarshaling
        internal TcpEndpoint(
            InputStream istr,
            Communicator communicator,
            Transport transport,
            Protocol protocol,
            bool mostDerived = true)
            : base(istr, communicator, protocol)
        {
            Transport = transport;
            if (protocol == Protocol.Ice1)
            {
                Timeout = TimeSpan.FromMilliseconds(istr.ReadInt());
                HasCompressionFlag = istr.ReadBool();
            }
            else if (mostDerived)
            {
                SkipUnknownOptions(istr, istr.ReadSize());
            }
        }

        // Constructor for URI parsing.
        internal TcpEndpoint(
            Communicator communicator,
            Transport transport,
            Protocol protocol,
            string host,
            ushort port,
            Dictionary<string, string> options,
            bool oaEndpoint)
            : base(communicator, protocol, host, port, options, oaEndpoint) => Transport = transport;

        internal TcpEndpoint(
            Communicator communicator,
            Transport transport,
            Protocol protocol,
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
                    Timeout = System.Threading.Timeout.InfiniteTimeSpan;
                }
                else
                {
                    try
                    {
                        Timeout = TimeSpan.FromMilliseconds(int.Parse(argument, CultureInfo.InvariantCulture));
                    }
                    catch (FormatException ex)
                    {
                        throw new FormatException($"invalid timeout value `{argument}' in endpoint `{endpointString}'",
                             ex);
                    }
                    if (Timeout <= TimeSpan.Zero)
                    {
                        throw new FormatException($"invalid timeout value `{argument}' in endpoint `{endpointString}'");
                    }
                }
                options.Remove("-t");
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
            new TcpConnector(this, addr, proxy);

        private protected override IPEndpoint Clone(string host, ushort port)
        {
            if (host == Host && port == Port)
            {
                return this;
            }
            else
            {
                return new TcpEndpoint(Communicator,
                                       Transport,
                                       Protocol,
                                       host,
                                       port,
                                       SourceAddress,
                                       Timeout,
                                       HasCompressionFlag);
            }
        }

        internal virtual ITransceiver CreateTransceiver(StreamSocket socket, string? adapterName)
        {
            if (IsSecure)
            {
                return new SslTransceiver(Communicator,
                                          new TcpTransceiver(socket),
                                          adapterName ?? Host,
                                          adapterName != null);
            }
            else
            {
                return new TcpTransceiver(socket);
            }
        }
    }
}
