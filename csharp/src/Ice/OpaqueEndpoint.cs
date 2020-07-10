//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>Describes an endpoint with a transport that the local Ice runtime does not know. The local Ice runtime
    /// cannot send a request to this endpoint; it can however marshal this endpoint (within a proxy) and send this
    /// proxy to another application that may know this transport.</summary>
    public sealed class OpaqueEndpoint : Endpoint
    {
        public ReadOnlyMemory<byte> Bytes { get; }

        public Encoding Encoding { get; }
        public override bool HasCompressionFlag => false;

        public override string Host { get; } = "";

        public override string? this[string option]
        {
            get
            {
                if (Protocol != Protocol.Ice1 && option == "options" && Options.Count > 0)
                {
                    // TODO: need to escape each option
                    return string.Join(",", Options);
                }
                else
                {
                    return null;
                }
            }
        }

        /// <summary>The options of this endpoint. Only applicable to the ice2 protocol.</summary>
        public IReadOnlyList<string> Options { get; } = Array.Empty<string>();
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
                    return Options.SequenceEqual(opaqueEndpoint.Options) && base.Equals(other);
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
                   var hash = new HashCode();
                   hash.Add(base.GetHashCode());
                   foreach (string option in Options)
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

        protected internal override void WriteOptions(OutputStream ostr) =>
            ostr.WriteSequence(Options, OutputStream.IceWriterFromString);

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

        // Constructor for ice2 or later unmarshaling
        internal OpaqueEndpoint(
            InputStream istr,
            Communicator communicator,
            Transport transport,
            Protocol protocol)
            : base(communicator, protocol)
        {
            Transport = transport;
            Encoding = istr.Encoding;
            Host = istr.ReadString();
            Port = istr.ReadUShort();
            Options = istr.ReadArray(1, InputStream.IceReaderIntoString);
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
            Encoding = Ice2Definitions.Encoding; // not used

            if (options.TryGetValue("options", out string? value))
            {
                Options = value.Split(","); // TODO: proper un-escaping
                options.Remove("options");
            }
        }
    }
}
