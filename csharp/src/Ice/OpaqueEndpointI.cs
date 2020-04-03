//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;

using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System;

namespace IceInternal
{
    internal sealed class OpaqueEndpointI : Endpoint
    {
        private EndpointType _type;
        private Ice.Encoding _rawEncoding;
        private byte[] _rawBytes;
        private int _hashCode = 0; // 0 is a special value that means not initialized.

        public OpaqueEndpointI(List<string> args)
        {
            _type = (EndpointType)(short)-1;
            _rawEncoding = Ice.Encoding.V1_1;
            _rawBytes = System.Array.Empty<byte>();

            InitWithOptions(args);

            if ((short)_type < 0)
            {
                throw new FormatException($"no -t option in endpoint {this}");
            }
            if (_rawBytes.Length == 0)
            {
                throw new FormatException($"no -v option in endpoint {this}");
            }
        }

        public OpaqueEndpointI(EndpointType type, Ice.Encoding encoding, byte[] rawBytes)
        {
            _type = type;
            _rawEncoding = encoding;
            _rawBytes = rawBytes;
        }

        //
        // Marshal the endpoint
        //
        public override void StreamWrite(Ice.OutputStream s)
        {
            s.StartEndpointEncapsulation(_rawEncoding);
            s.WriteSpan(_rawBytes.AsSpan());
            s.EndEndpointEncapsulation();
        }

        public override void StreamWriteImpl(Ice.OutputStream s) => Debug.Assert(false);

        //
        // Convert the endpoint to its string form
        //
        public override string ToString()
        {
            string val = System.Convert.ToBase64String(_rawBytes);
            short typeNum = (short)_type;
            return $"opaque -t {typeNum.ToString(CultureInfo.InvariantCulture)} -e {_rawEncoding} -v {val}";
        }

        private sealed class InfoI : Ice.OpaqueEndpointInfo
        {
            public InfoI(EndpointType type, Ice.Encoding rawEncoding, byte[] rawBytes) :
                base(null, -1, false, rawEncoding, rawBytes) => _type = type;

            public override EndpointType Type() => _type;

            public override bool Datagram() => false;

            public override bool Secure() => false;

            private readonly EndpointType _type;
        }

        //
        // Return the endpoint information.
        //
        public override Ice.EndpointInfo GetInfo() => new InfoI(_type, _rawEncoding, _rawBytes);

        //
        // Return the endpoint type
        //
        public override EndpointType Type() => _type;

        //
        // Return the transport name;
        //
        public override string Transport() => "opaque";

        //
        // Return the timeout for the endpoint in milliseconds. 0 means
        // non-blocking, -1 means no timeout.
        //
        public override int Timeout() => -1;

        //
        // Return a new endpoint with a different timeout value, provided
        // that timeouts are supported by the endpoint. Otherwise the same
        // endpoint is returned.
        //
        public override Endpoint Timeout(int t) => this;

        public override string ConnectionId() => "";

        //
        // Return a new endpoint with a different connection id.
        //
        public override Endpoint ConnectionId(string id) => this;

        //
        // Return true if the endpoints support bzip2 compress, or false
        // otherwise.
        //
        public override bool Compress() => false;

        //
        // Return a new endpoint with a different compression value,
        // provided that compression is supported by the
        // endpoint. Otherwise the same endpoint is returned.
        //
        public override Endpoint Compress(bool compress) => this;

        //
        // Return true if the endpoint is datagram-based.
        //
        public override bool Datagram() => false;

        //
        // Return true if the endpoint is secure.
        //
        public override bool Secure() => false;

        //
        // Get the encoded endpoint.
        //
        public byte[] RawBytes() => _rawBytes;

        //
        // Return a server side transceiver for this endpoint, or null if a
        // transceiver can only be created by an acceptor.
        //
        public override ITransceiver? Transceiver() => null;

        //
        // Return connectors for this endpoint, or empty list if no connector
        // is available.
        //
        public override void ConnectorsAsync(Ice.EndpointSelectionType endSel, IEndpointConnectors callback) =>
            callback.Connectors(new List<IConnector>());

        //
        // Return an acceptor for this endpoint, or null if no acceptors
        // is available.
        //
        public override IAcceptor? Acceptor(string adapterName) => null;

        //
        // Expand endpoint out in to separate endpoints for each local
        // host if listening on INADDR_ANY on server side or if no host
        // was specified on client side.
        //
        public override List<Endpoint> ExpandIfWildcard() => new List<Endpoint> { this };

        public override List<Endpoint> ExpandHost(out Endpoint? publishedEndpoint)
        {
            publishedEndpoint = null;
            return new List<Endpoint>() { this };
        }

        //
        // Check whether the endpoint is equivalent to another one.
        //
        public override bool Equivalent(Endpoint endpoint) => false;

        public override int GetHashCode()
        {
            if (_hashCode != 0)
            {
                return _hashCode;
            }
            else
            {
               int hashCode = HashCode.Combine(_type, _rawEncoding, _rawBytes);
               if (hashCode == 0)
               {
                   hashCode = 1;
               }
               _hashCode = hashCode;
               return _hashCode;
            }
        }

        public override string Options()
        {
            string s = "";
            if ((short)_type > -1)
            {
                s += " -t " + _type;
            }
            s += " -e " + _rawEncoding.ToString();
            if (_rawBytes.Length > 0)
            {
                s += " -v " + System.Convert.ToBase64String(_rawBytes);
            }
            return s;
        }

        //
        // Compare endpoints for sorting purposes
        //
        public override int CompareTo(Endpoint obj)
        {
            if (!(obj is OpaqueEndpointI))
            {
                return Type() < obj.Type() ? -1 : 1;
            }

            var p = (OpaqueEndpointI)obj;
            if (this == p)
            {
                return 0;
            }

            if (_type < p._type)
            {
                return -1;
            }
            else if (p._type < _type)
            {
                return 1;
            }

            if (_rawEncoding.Major < p._rawEncoding.Major)
            {
                return -1;
            }
            else if (p._rawEncoding.Major < _rawEncoding.Major)
            {
                return 1;
            }

            if (_rawEncoding.Minor < p._rawEncoding.Minor)
            {
                return -1;
            }
            else if (p._rawEncoding.Minor < _rawEncoding.Minor)
            {
                return 1;
            }

            if (_rawBytes.Length < p._rawBytes.Length)
            {
                return -1;
            }
            else if (p._rawBytes.Length < _rawBytes.Length)
            {
                return 1;
            }
            for (int i = 0; i < _rawBytes.Length; i++)
            {
                if (_rawBytes[i] < p._rawBytes[i])
                {
                    return -1;
                }
                else if (p._rawBytes[i] < _rawBytes[i])
                {
                    return 1;
                }
            }

            return 0;
        }

        protected override bool CheckOption(string option, string? argument, string endpoint)
        {
            switch (option[1])
            {
                case 't':
                    {
                        if ((short)_type > -1)
                        {
                            throw new FormatException($"multiple -t options in endpoint {endpoint}");
                        }

                        if (argument == null)
                        {
                            throw new FormatException($"no argument provided for -t option in endpoint {endpoint}");
                        }

                        short t;
                        try
                        {
                            t = short.Parse(argument, CultureInfo.InvariantCulture);
                        }
                        catch (FormatException ex)
                        {
                            throw new FormatException($"invalid type value `{argument}' in endpoint {endpoint}", ex);
                        }

                        if (t < 0)
                        {
                            throw new FormatException($"type value `{argument}' out of range in endpoint {endpoint}");
                        }

                        _type = (EndpointType)t;
                        return true;
                    }

                case 'v':
                    {
                        if (_rawBytes.Length > 0)
                        {
                            throw new FormatException($"multiple -v options in endpoint {endpoint}");
                        }
                        if (argument == null)
                        {
                            throw new FormatException($"no argument provided for -v option in endpoint {endpoint}");
                        }

                        try
                        {
                            _rawBytes = Convert.FromBase64String(argument);
                        }
                        catch (FormatException ex)
                        {
                            throw new FormatException("Invalid Base64 input in endpoint {endpoint}", ex);
                        }

                        return true;
                    }

                case 'e':
                    {
                        if (argument == null)
                        {
                            throw new FormatException($"no argument provided for -e option in endpoint {endpoint}");
                        }

                        try
                        {
                            _rawEncoding = Ice.Encoding.Parse(argument);
                        }
                        catch (FormatException ex)
                        {
                            throw new FormatException($"invalid encoding version `{argument}' in endpoint {endpoint}", ex);
                        }
                        return true;
                    }

                default:
                    {
                        return false;
                    }
            }
        }
    }
}
