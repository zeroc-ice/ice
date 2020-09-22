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
    /// <summary>Describes an ice1 endpoint that the associated communicator cannot use, typically because it does not
    /// implement the endpoint's transport. The communicator can marshal a proxy with such an endpoint and send it to
    /// another Ice application that may know/decode this endpoint. This class is used only with the ice1 protocol.
    /// </summary>
    internal sealed class OpaqueEndpoint : Endpoint
    {
        public override string Host => "";

        public override string? this[string option] =>
            option switch
            {
                "transport" => ((short)Transport).ToString(CultureInfo.InvariantCulture),
                "value" => Value.IsEmpty ? null : Convert.ToBase64String(Value.Span),
                "value-encoding" => ValueEncoding.ToString(),
                _ => null,
            };

        public override string Scheme => "opaque";
        public override Transport Transport { get; }

        protected internal override ushort DefaultPort => 0;
        protected internal override bool HasOptions => true;

        internal ReadOnlyMemory<byte> Value { get; }

        internal Encoding ValueEncoding { get; }

        private int _hashCode; // 0 is a special value that means not initialized.

        public override IAcceptor Acceptor(IConnectionManager manager, ObjectAdapter adapter) =>
            throw new InvalidOperationException();

        public override bool Equals(Endpoint? other)
        {
            if (ReferenceEquals(this, other))
            {
                return true;
            }
            return other is OpaqueEndpoint opaqueEndpoint &&
                ValueEncoding == opaqueEndpoint.ValueEncoding &&
                Value.Span.SequenceEqual(opaqueEndpoint.Value.Span) &&
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
                int hashCode = HashCode.Combine(base.GetHashCode(), ValueEncoding, Value);
                if (hashCode == 0)
                {
                    hashCode = 1;
                }
                _hashCode = hashCode;
                return _hashCode;
            }
        }

        public override bool IsLocal(Endpoint endpoint) => false;

        protected internal override void WriteOptions(OutputStream ostr)
        {
            Debug.Assert(false);
            throw new NotImplementedException("cannot write the options of an opaque endpoint");
        }

        public override ValueTask<IEnumerable<IConnector>> ConnectorsAsync(
            EndpointSelectionType selType,
            CancellationToken cancel) =>
            new ValueTask<IEnumerable<IConnector>>(new List<IConnector>());

        public override Connection CreateDatagramServerConnection(ObjectAdapter adapter) =>
            throw new InvalidOperationException();

        public override IEnumerable<Endpoint> ExpandHost(out Endpoint? publishedEndpoint)
        {
            publishedEndpoint = null;
            return new Endpoint[] { this };
        }

        public override IEnumerable<Endpoint> ExpandIfWildcard() => new Endpoint[] { this };

        protected internal override void AppendOptions(StringBuilder sb, char optionSeparator)
        {
            sb.Append(" -t ");
            sb.Append(((short)Transport).ToString(CultureInfo.InvariantCulture));

            sb.Append(" -e ");
            sb.Append(ValueEncoding.ToString());
            if (!Value.IsEmpty)
            {
                sb.Append(" -v ");
                sb.Append(Convert.ToBase64String(Value.Span));
            }
        }

        // Constructor for parsing the ice1 string format.
        internal OpaqueEndpoint(
            Communicator communicator,
            Dictionary<string, string?> options,
            string endpointString)
            : base(communicator, Protocol.Ice1)
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

            // the caller deals with the remaining options, if any
        }

        // Constructor for unmarshaling.
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
    }
}
