//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>Describes an endpoint with a transport that the associated communicator does not know. See also
    /// <see cref="Communicator.IceAddEndpointFactory"/>. The communicator cannot send a request to this endpoint; it
    /// can however marshal this endpoint (within a proxy) and send this proxy to another application that may know this
    /// transport.</summary>
    internal sealed class OpaqueEndpoint : Endpoint
    {
        public override bool HasCompressionFlag => false;

        public override string Host { get; } = "";

        public override string? this[string option]
        {
            get
            {
                if (Protocol == Protocol.Ice1)
                {
                    return option switch
                    {
                        "value" => Value.Length > 0 ? Convert.ToBase64String(Value.Span) : null,
                        "value-encoding" => ValueEncoding.ToString(),
                        _ => null,
                    };
                }
                else if (option == "option" && _options.Count > 0)
                {
                    // We percent-escape each option, just like in their URI form.
                    return string.Join(",", _options.Select(s => Uri.EscapeDataString(s)));
                }
                else
                {
                    return null;
                }
            }
        }

        public override ushort Port { get; }

        public override Transport Transport { get; }

        // TODO: should be Protocol == Protocol.Ice1 ? "opaque" : base.TransportName;
        public override string TransportName => "opaque";

        internal ReadOnlyMemory<byte> Value { get; }

        internal Encoding ValueEncoding { get; }

        private int _hashCode = 0; // 0 is a special value that means not initialized.

        /// <summary>The options of this endpoint. Only applicable to the ice2 protocol.</summary>
        private readonly IReadOnlyList<string> _options = Array.Empty<string>();

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
                    return ValueEncoding == opaqueEndpoint.ValueEncoding &&
                        Value.Span.SequenceEqual(opaqueEndpoint.Value.Span) &&
                        base.Equals(other);
                }
                else
                {
                    return _options.SequenceEqual(opaqueEndpoint._options) && base.Equals(other);
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
                    hashCode = HashCode.Combine(base.GetHashCode(), ValueEncoding, Value);
               }
               else
               {
                   var hash = new HashCode();
                   hash.Add(base.GetHashCode());
                   foreach (string option in _options)
                   {
                       hash.Add(option);
                   }
                   hashCode = hash.ToHashCode();
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
            sb.Append(ValueEncoding.ToString());
            if (Value.Length > 0)
            {
                sb.Append(" -v ");
                sb.Append(Convert.ToBase64String(Value.Span));
            }
            return sb.ToString();
        }

        public override bool IsLocal(Endpoint endpoint) => false;

        protected internal override void WriteOptions(OutputStream ostr) =>
            ostr.WriteSequence(_options, OutputStream.IceWriterFromString);

        public override void IceWritePayload(OutputStream ostr)
        {
            Debug.Assert(false);
            throw new NotImplementedException("cannot write the payload for an opaque endpoint");
        }

        public override Endpoint NewTimeout(TimeSpan t) => this;
        public override Endpoint NewCompressionFlag(bool compress) => this;

        public override ValueTask<IEnumerable<IConnector>> ConnectorsAsync(EndpointSelectionType _) =>
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
                    ValueEncoding = Encoding.Parse(argument);
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
                ValueEncoding = Encoding.V1_1;
            }

            if (options.TryGetValue("-v", out argument))
            {
                if (argument == null)
                {
                    throw new FormatException($"no argument provided for -v option in endpoint `{endpointString}'");
                }

                try
                {
                    Value = Convert.FromBase64String(argument);
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
            Encoding payloadEncoding,
            byte[] bytes)
            : base(communicator, Protocol.Ice1)
        {
            Transport = transport;
            ValueEncoding = payloadEncoding;
            Value = bytes;
        }

        // Constructor for ice2 or later unmarshaling
        internal OpaqueEndpoint(
            InputStream istr,
            Communicator communicator,
            Transport transport,
            Protocol protocol)
            : base(communicator, protocol)
        {
            Transport = transport;
            ValueEncoding = istr.Encoding;
            Host = istr.ReadString();
            Port = istr.ReadUShort();
            _options = istr.ReadArray(1, InputStream.IceReaderIntoString);
        }

        // Constructor for URI parsing with ice2 or later.
        internal OpaqueEndpoint(
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
            ValueEncoding = Ice2Definitions.Encoding; // not used

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
