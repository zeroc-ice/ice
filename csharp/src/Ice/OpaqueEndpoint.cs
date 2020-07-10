//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Diagnostics;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>An opaque endpoint is an endpoint with a transport that the local Ice runtime does not know. As a
    /// result, the local Ice runtime cannot send a request to this endpoint. It can however marshal this endpoint
    /// (within a proxy) and send this proxy to another application that knows this transport.</summary>
    public sealed class OpaqueEndpoint : Endpoint
    {
        public override string? this[string option]
        {
            get => Options.TryGetValue(option, out string? value) ? value : null;
        }

        public ReadOnlyMemory<byte> Bytes { get; }

        public Encoding Encoding { get; }
        public override bool HasCompressionFlag => false;

        public override string Host { get; } = "";

        /// <summary>The options of this endpoint. Only applicable to the ice2 protocol.</summary>
        public IReadOnlyDictionary<string, string> Options { get; } = ImmutableDictionary<string, string>.Empty;
        public override ushort Port { get; }

        public override Transport Transport { get; }
        public override string TransportName => "opaque"; // TODO: is this always correct?

        private int _hashCode = 0; // 0 is a special value that means not initialized.

        public override bool Equals(Endpoint? other)
        {
            if (ReferenceEquals(this, other))
            {
                return true;
            }
            if (other is OpaqueEndpoint opaqueEndpoint)
            {
                if (Protocol == Protocol.Ice1)
                {
                    return Encoding == opaqueEndpoint.Encoding &&
                        Bytes.Span.SequenceEqual(opaqueEndpoint.Bytes.Span) &&
                        base.Equals(other);
                }
                else
                {
                    return Options.DictionaryEqual(opaqueEndpoint.Options) && base.Equals(other);
                }
            }
            else
            {
                return false;
            }
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
               if (Protocol == Protocol.Ice1)
               {
                    hashCode = HashCode.Combine(base.GetHashCode(), Encoding, Bytes);
               }
               else
               {
                   hashCode = HashCode.Combine(base.GetHashCode(), Options.GetDictionaryHashCode());
               }
               if (hashCode == 0)
               {
                   hashCode = 1;
               }
               _hashCode = hashCode;
               return _hashCode;
            }
        }

        public override string OptionsToString()
        {
            var sb = new StringBuilder(" -t ");
            sb.Append(((short)Transport).ToString(CultureInfo.InvariantCulture));

            sb.Append(" -e ");
            sb.Append(Encoding.ToString());
            if (Bytes.Length > 0)
            {
                sb.Append(" -v ");
                sb.Append(System.Convert.ToBase64String(Bytes.Span));
            }
            return sb.ToString();
        }

        public override bool IsLocal(Endpoint endpoint) => false;

        public override string ToString()
        {
            string val = System.Convert.ToBase64String(Bytes.Span);
            short transportNum = (short)Transport;
            return $"opaque -t {transportNum.ToString(CultureInfo.InvariantCulture)} -e {Encoding} -v {val}";
        }

        public override void IceWritePayload(OutputStream ostr)
        {
            Debug.Assert(false);
            throw new NotImplementedException("cannot write the payload for an opaque endpoint");
        }

        public override Endpoint NewTimeout(int t) => this;
        public override Endpoint NewCompressionFlag(bool compress) => this;

        public override ValueTask<IEnumerable<IConnector>> ConnectorsAsync(EndpointSelectionType endSel) =>
            new ValueTask<IEnumerable<IConnector>>(new List<IConnector>());

        public override IEnumerable<Endpoint> ExpandHost(out Endpoint? publishedEndpoint)
        {
            publishedEndpoint = null;
            return new Endpoint[] { this };
        }

        public override IEnumerable<Endpoint> ExpandIfWildcard() => new Endpoint[] { this };

        public override IAcceptor? GetAcceptor(string adapterName) => null;
        public override ITransceiver? GetTransceiver() => null;

        // Constructor for parsing the old format (ice1 only).
        internal OpaqueEndpoint(
            Communicator communicator,
            Protocol protocol,                    // TODO: remove protocol
            Dictionary<string, string?> options,
            string endpointString)
            : base(communicator, protocol)
        {
            if (options.TryGetValue("-t", out string? argument))
            {
                if (argument == null)
                {
                    throw new FormatException($"no argument provided for -t option in endpoint `{endpointString}'");
                }
                short t;
                try
                {
                    t = short.Parse(argument, CultureInfo.InvariantCulture);
                }
                catch (FormatException ex)
                {
                    throw new FormatException(
                        $"invalid transport value `{argument}' in endpoint `{endpointString}'", ex);
                }

                if (t < 0)
                {
                    throw new FormatException(
                        $"transport value `{argument}' out of range in endpoint `{endpointString}'");
                }

                Transport = (Transport)t;
                options.Remove("-t");
            }
            else
            {
                throw new FormatException($"no -t option in endpoint `{endpointString}'");
            }

            if (options.TryGetValue("-e", out argument))
            {
                if (argument == null)
                {
                    throw new FormatException($"no argument provided for -e option in endpoint `{endpointString}'");
                }
                try
                {
                    Encoding = Encoding.Parse(argument);
                }
                catch (FormatException ex)
                {
                    throw new FormatException($"invalid encoding version `{argument}' in endpoint `{endpointString}'",
                        ex);
                }
                options.Remove("-e");
            }
            else
            {
                Encoding = Encoding.Latest; // TODO: should be the default encoding for this communicator
            }

            if (options.TryGetValue("-v", out argument))
            {
                if (argument == null)
                {
                    throw new FormatException($"no argument provided for -v option in endpoint `{endpointString}'");
                }

                try
                {
                    Bytes = Convert.FromBase64String(argument);
                }
                catch (FormatException ex)
                {
                    throw new FormatException($"invalid Base64 input in endpoint `{endpointString}'", ex);
                }
                options.Remove("-v");
            }
            else
            {
                throw new FormatException($"no -v option in endpoint `{endpointString}'");
            }

            // the caller deals with remaining options, if any
        }

        // Constructor for ice1 unmarshaling.
        internal OpaqueEndpoint(
            Communicator communicator,
            Transport transport,
            Encoding encoding,
            byte[] bytes)
            : base(communicator, Protocol.Ice1)
        {
            Transport = transport;
            Encoding = encoding;
            Bytes = bytes;
        }

        // Constructor for ice2 unmarshaling or URI parsing with ice2.
        internal OpaqueEndpoint(
            Communicator communicator,
            Transport transport,
            string host,
            ushort port,
            IReadOnlyDictionary<string, string> options)
            : base(communicator, Protocol.Ice2)
        {
            Transport = transport;
            Host = host;
            Port = port;
            Options = options;
            Encoding = Ice2Definitions.Encoding; // not used with ice2
            Bytes = default; // not used with ice2
        }
    }
}
