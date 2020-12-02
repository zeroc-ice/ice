// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Linq;
using System.Runtime.InteropServices;
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
        public override string? this[string option] =>
            option switch
            {
                "transport" => ((short)Transport).ToString(CultureInfo.InvariantCulture),
                "value" => Value.IsEmpty ? null : Convert.ToBase64String(Value.Span),
                "value-encoding" => ValueEncoding.ToString(),
                _ => null,
            };

        public override string Scheme => "opaque";

        protected internal override ushort DefaultPort => 0;
        protected internal override bool HasOptions => true;

        internal ReadOnlyMemory<byte> Value { get; }

        internal Encoding ValueEncoding { get; }

        private int _hashCode; // 0 is a special value that means not initialized.

        public override IAcceptor Acceptor(ObjectAdapter adapter) =>
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
                int hashCode = HashCode.Combine(
                    base.GetHashCode(),
                    ValueEncoding,
                    MemoryMarshal.ToEnumerable(Value).GetSequenceHashCode());

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

        public override Connection CreateDatagramServerConnection(ObjectAdapter adapter) =>
            throw new InvalidOperationException();

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

        protected internal override Task<Connection> ConnectAsync(
            NonSecure preferNonSecure,
            object? label,
            CancellationToken cancel) =>
            throw new NotSupportedException("cannot create a connection to an opaque endpoint");

        protected internal override Endpoint GetPublishedEndpoint(string serverName) =>
            throw new NotSupportedException("cannot create published endpoint for opaque endpoint");

        internal static OpaqueEndpoint Create(
            Transport transport,
            Encoding valueEncoding,
            ReadOnlyMemory<byte> value,
            Communicator communicator) =>
            new OpaqueEndpoint(new EndpointData(transport, host: "", port: 0, Array.Empty<string>()),
                               valueEncoding,
                               value,
                               communicator);

        internal static OpaqueEndpoint Parse(
            Dictionary<string, string?> options,
            Communicator communicator,
            string endpointString)
        {
            Transport transport;

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

                transport = (Transport)t;
                options.Remove("-t");
            }
            else
            {
                throw new FormatException($"no -t option in endpoint `{endpointString}'");
            }

            Encoding valueEncoding;

            if (options.TryGetValue("-e", out argument))
            {
                if (argument == null)
                {
                    throw new FormatException($"no argument provided for -e option in endpoint `{endpointString}'");
                }
                try
                {
                    valueEncoding = Encoding.Parse(argument);
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
                valueEncoding = Encoding.V11;
            }

            ReadOnlyMemory<byte> value;

            if (options.TryGetValue("-v", out argument))
            {
                if (argument == null)
                {
                    throw new FormatException($"no argument provided for -v option in endpoint `{endpointString}'");
                }

                try
                {
                    value = Convert.FromBase64String(argument);
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

            return Create(transport, valueEncoding, value, communicator);
        }

        private OpaqueEndpoint(
            EndpointData data,
            Encoding valueEncoding,
            ReadOnlyMemory<byte> value,
            Communicator communicator)
            : base(data, communicator, Protocol.Ice1)
        {
            ValueEncoding = valueEncoding;
            Value = value;
        }
    }
}
