//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>Describes an endpoint with a transport or protocol that the associated communicator does not implement.
    /// The communicator cannot send a request to this endpoint; it can however marshal this endpoint (within a proxy)
    /// and send this proxy to another application that may know this transport. This class is used only for protocol
    /// ice2 or greater.</summary>
    internal sealed class UniversalEndpoint : Endpoint
    {
        public override string Host { get; }

        public override string? this[string option] =>
            option switch
            {
                "option" => _options.Count > 0 ? string.Join(",", _options.Select(s => Uri.EscapeDataString(s))) : null,
                "transport" => TransportName,
                _ => null
            };

        public override ushort Port { get; }

        public override string Scheme => "ice+universal";
        public override Transport Transport { get; }

        protected internal override ushort DefaultPort => DefaultUniversalPort;
        protected internal override bool HasOptions => true;

        internal const ushort DefaultUniversalPort = 0;

        private int _hashCode; // 0 is a special value that means not initialized.

        private readonly IReadOnlyList<string> _options = Array.Empty<string>();

        public override bool Equals(Endpoint? other)
        {
            if (ReferenceEquals(this, other))
            {
                return true;
            }
            return other is UniversalEndpoint universalEndpoint &&
                _options.SequenceEqual(universalEndpoint._options) &&
                 base.Equals(other);
        }

        public override int GetHashCode()
        {
            if (_hashCode != 0)
            {
                return _hashCode;
            }
            else
            {
                int hashCode;
                var hash = new HashCode();
                hash.Add(base.GetHashCode());
                foreach (string option in _options)
                {
                    hash.Add(option);
                }
                hashCode = hash.ToHashCode();

                if (hashCode == 0)
                {
                    hashCode = 1;
                }
                _hashCode = hashCode;
                return _hashCode;
            }
        }

        public override bool IsLocal(Endpoint endpoint) => false;

        public override ValueTask<IEnumerable<IConnector>> ConnectorsAsync(EndpointSelectionType _) =>
            new ValueTask<IEnumerable<IConnector>>(new List<IConnector>());

        public override Connection CreateConnection(
            IConnectionManager manager,
            ITransceiver? transceiver,
            IConnector? connector,
            string connectionId,
            ObjectAdapter? adapter) => null!;

        public override IEnumerable<Endpoint> ExpandHost(out Endpoint? publishedEndpoint)
        {
            publishedEndpoint = null;
            return new Endpoint[] { this };
        }

        public override IEnumerable<Endpoint> ExpandIfWildcard() => new Endpoint[] { this };

        public override ITransceiver? GetTransceiver() => null;

        protected internal override void AppendOptions(StringBuilder sb, char optionSeparator)
        {
            sb.Append("transport=");
            sb.Append(TransportName);

            if (_options.Count > 0)
            {
                sb.Append(optionSeparator);
                sb.Append("option=");
                for (int i = 0; i < _options.Count; ++i)
                {
                    if (i > 0)
                    {
                        sb.Append(',');
                    }
                    sb.Append(Uri.EscapeDataString(_options[i]));
                }
            }
        }

        protected internal override void WriteOptions(OutputStream ostr) =>
            ostr.WriteSequence(_options, OutputStream.IceWriterFromString);

        // Constructor for unmarshaling.
        internal UniversalEndpoint(
            InputStream istr,
            Communicator communicator,
            Transport transport,
            Protocol protocol)
            : base(communicator, protocol)
        {
            Debug.Assert(Protocol != Protocol.Ice1);
            Transport = transport;
            Host = istr.ReadString();
            Port = istr.ReadUShort();
            _options = istr.ReadArray(1, InputStream.IceReaderIntoString);
        }

        // Constructor for URI parsing.
        internal UniversalEndpoint(
            Communicator communicator,
            Transport transport,
            Protocol protocol,
            string host,
            ushort port,
            Dictionary<string, string> options)
            : base(communicator, protocol)
        {
            Transport = transport;
            Host = host;
            Port = port;

            if (options.TryGetValue("option", out string? value))
            {
                // Each option must be percent-escaped; we hold it in memory unescaped, and later marshal it unescaped.
                // For example, a WS endpoint resource option can be provided "double-escaped", with
                // `/` replaced by %2F and %20 (space) escaped as %2520; this unescaping would result in
                // the memory resource value being "singled-escaped".
                _options = value.Split(",").Select(s => Uri.UnescapeDataString(s)).ToList();
                options.Remove("option");
            }
        }
    }
}
