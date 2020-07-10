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
    internal class TcpEndpoint : IPEndpoint
    {
        public override bool HasCompressionFlag { get; } = false;
        public override bool IsDatagram => false;
        public override bool IsSecure => Transport == Transport.SSL;
        public override int Timeout { get; } = -1;
        public override Transport Transport { get; }

        private int _hashCode = 0;

        public override bool Equals(Endpoint? other)
        {
            if (ReferenceEquals(this, other))
            {
                return true;
            }

            if (Protocol == Protocol.Ice1)
            {
                return other is TcpEndpoint tcpEndpoint &&
                    HasCompressionFlag == tcpEndpoint.HasCompressionFlag &&
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
                    hashCode = HashCode.Combine(base.GetHashCode(), HasCompressionFlag, Timeout);
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

        public override string OptionsToString()
        {
            if (Protocol == Protocol.Ice1) // TODO: temporary, Protocol should always be ice1
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
            else
            {
                return base.OptionsToString();
            }
        }

        public override void IceWritePayload(OutputStream ostr)
        {
            Debug.Assert(Protocol == Protocol.Ice1);
            base.IceWritePayload(ostr);
            ostr.WriteInt(Timeout);
            ostr.WriteBool(HasCompressionFlag);
        }

        public override Endpoint NewTimeout(int timeout) =>
            timeout == Timeout ? this : CreateIPEndpoint(Host, Port, HasCompressionFlag, timeout);

        public override IAcceptor GetAcceptor(string adapterName) =>
            new TcpAcceptor(this, Communicator, Host, Port, adapterName);
        public override ITransceiver? GetTransceiver() => null;

        internal TcpEndpoint(
            Communicator communicator,
            Transport transport,
            Protocol protocol,
            string host,
            ushort port,
            IPAddress? sourceAddress,
            int timeout,
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
                Timeout = istr.ReadInt();
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
            bool oaEndpoint,
            string endpointString)
            : base(communicator, protocol, host, port, options, oaEndpoint)
        {
            Transport = transport;
            if (protocol == Protocol.Ice1)
            {
                if (options.TryGetValue("compress", out string? value))
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

                if (options.TryGetValue("timeout", out value))
                {
                    if (value == "infinite")
                    {
                        Timeout = -1;
                    }
                    else
                    {
                        Timeout = int.Parse(value, CultureInfo.InvariantCulture);
                        if (Timeout < -1)
                        {
                            throw new FormatException(
                                $"invalid timeout value `{value}' in endpoint `{endpointString}'");
                        }
                    }
                }
            }
        }

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
                    if (Timeout < -1)
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
                             Timeout);

        private protected override IPEndpoint CreateIPEndpoint(
            string host,
            ushort port,
            bool compressionFlag,
            int timeout) =>
            new TcpEndpoint(Communicator,
                            Transport,
                            Protocol,
                            host,
                            port,
                            SourceAddress,
                            timeout,
                            compressionFlag);

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
