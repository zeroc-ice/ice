//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using IceInternal;

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Linq;
using System.Text;

namespace Ice
{
    public sealed class OpaqueEndpoint : Endpoint
    {
        public ReadOnlyMemory<byte> Bytes { get; }
        public override string ConnectionId => "";
        public Ice.Encoding Encoding { get; }
        public override bool HasCompressionFlag => false;
        public override bool IsDatagram => false;
        public override bool IsSecure => false;
        public override string Name => "opaque";

        public override int Timeout => -1;
        public override EndpointType Type { get; }

        private int _hashCode = 0; // 0 is a special value that means not initialized.

        public override bool Equals(Endpoint? other)
        {
            if (ReferenceEquals(this, other))
            {
                return true;
            }
            if (other is OpaqueEndpoint opaqueEndpoint)
            {
                if (Type != opaqueEndpoint.Type)
                {
                    return false;
                }
                if (Encoding != opaqueEndpoint.Encoding)
                {
                    return false;
                }
                if (!Bytes.Span.SequenceEqual(opaqueEndpoint.Bytes.Span))
                {
                    return false;
                }
                return true;
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
               int hashCode = HashCode.Combine(Type, Encoding, Bytes);
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
            sb.Append(((short)Type).ToString(CultureInfo.InvariantCulture));

            sb.Append(" -e ");
            sb.Append(Encoding.ToString());
            if (Bytes.Length > 0)
            {
                sb.Append(" -v ");
                sb.Append(System.Convert.ToBase64String(Bytes.Span));
            }
            return sb.ToString();
        }

        public override bool Equivalent(Endpoint endpoint) => false;

        public override string ToString()
        {
            string val = System.Convert.ToBase64String(Bytes.Span);
            short typeNum = (short)Type;
            return $"opaque -t {typeNum.ToString(CultureInfo.InvariantCulture)} -e {Encoding} -v {val}";
        }

        public override void IceWritePayload(Ice.OutputStream ostr)
        {
            Debug.Assert(false);
            throw new NotImplementedException("cannot write the payload for an opaque endpoint");
        }

        public override Endpoint NewTimeout(int t) => this;

        public override Endpoint NewConnectionId(string id) => this;
        public override Endpoint NewCompressionFlag(bool compress) => this;

        public override void ConnectorsAsync(Ice.EndpointSelectionType endSel, IEndpointConnectors callback) =>
            callback.Connectors(new List<IConnector>());

        public override IEnumerable<Endpoint> ExpandHost(out Endpoint? publishedEndpoint)
        {
            publishedEndpoint = null;
            return new Endpoint[] { this };
        }

        public override IEnumerable<Endpoint> ExpandIfWildcard() => new Endpoint[] { this };

        public override IAcceptor? GetAcceptor(string adapterName) => null;
        public override ITransceiver? GetTransceiver() => null;

        internal OpaqueEndpoint(string endpointString, Dictionary<string, string?> options)
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
                    throw new FormatException($"invalid type value `{argument}' in endpoint `{endpointString}'", ex);
                }

                if (t < 0)
                {
                    throw new FormatException($"type value `{argument}' out of range in endpoint `{endpointString}'");
                }

                Type = (EndpointType)t;
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
                    Encoding = Ice.Encoding.Parse(argument);
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

        internal OpaqueEndpoint(EndpointType type, Ice.Encoding encoding, byte[] bytes)
        {
            Type = type;
            Encoding = encoding;
            Bytes = bytes;
        }
    }
}
