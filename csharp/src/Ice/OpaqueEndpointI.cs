//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Globalization;
    using System;

    internal sealed class OpaqueEndpointI : Endpoint
    {
        public OpaqueEndpointI(List<string> args)
        {
            _type = -1;
            _rawEncoding = Ice.Util.Encoding_1_0;
            _rawBytes = System.Array.Empty<byte>();

            InitWithOptions(args);

            if (_type < 0)
            {
                throw new FormatException($"no -t option in endpoint {this}");
            }
            if (_rawBytes.Length == 0)
            {
                throw new FormatException($"no -v option in endpoint {this}");
            }
        }

        public OpaqueEndpointI(short type, Ice.InputStream s)
        {
            _type = type;
            _rawEncoding = s.Encoding;
            int sz = s.GetEncapsulationSize();
            _rawBytes = new byte[sz];
            s.ReadBlob(_rawBytes);
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

        public override void StreamWriteImpl(Ice.OutputStream s)
        {
            Debug.Assert(false);
        }

        //
        // Convert the endpoint to its string form
        //
        public override string ToString()
        {
            string val = System.Convert.ToBase64String(_rawBytes);
            return "opaque -t " + _type + " -e " + Ice.Util.EncodingToString(_rawEncoding) + " -v " + val;
        }

        private sealed class InfoI : Ice.OpaqueEndpointInfo
        {
            public InfoI(short type, Ice.Encoding rawEncoding, byte[] rawBytes) :
                base(null, -1, false, rawEncoding, rawBytes)
            {
                _type = type;
            }

            public override short Type()
            {
                return _type;
            }

            public override bool Datagram()
            {
                return false;
            }

            public override bool Secure()
            {
                return false;
            }

            private readonly short _type;
        }

        //
        // Return the endpoint information.
        //
        public override Ice.EndpointInfo GetInfo()
        {
            return new InfoI(_type, _rawEncoding, _rawBytes);
        }

        //
        // Return the endpoint type
        //
        public override short Type()
        {
            return _type;
        }

        //
        // Return the transport name;
        //
        public override string Transport()
        {
            return "opaque";
        }

        //
        // Return the timeout for the endpoint in milliseconds. 0 means
        // non-blocking, -1 means no timeout.
        //
        public override int Timeout()
        {
            return -1;
        }

        //
        // Return a new endpoint with a different timeout value, provided
        // that timeouts are supported by the endpoint. Otherwise the same
        // endpoint is returned.
        //
        public override Endpoint Timeout(int t)
        {
            return this;
        }

        public override string ConnectionId()
        {
            return "";
        }

        //
        // Return a new endpoint with a different connection id.
        //
        public override Endpoint ConnectionId(string id)
        {
            return this;
        }

        //
        // Return true if the endpoints support bzip2 compress, or false
        // otherwise.
        //
        public override bool Compress()
        {
            return false;
        }

        //
        // Return a new endpoint with a different compression value,
        // provided that compression is supported by the
        // endpoint. Otherwise the same endpoint is returned.
        //
        public override Endpoint Compress(bool compress)
        {
            return this;
        }

        //
        // Return true if the endpoint is datagram-based.
        //
        public override bool Datagram()
        {
            return false;
        }

        //
        // Return true if the endpoint is secure.
        //
        public override bool Secure()
        {
            return false;
        }

        //
        // Get the encoded endpoint.
        //
        public byte[] rawBytes()
        {
            return _rawBytes;
        }

        //
        // Return a server side transceiver for this endpoint, or null if a
        // transceiver can only be created by an acceptor.
        //
        public override ITransceiver? Transceiver()
        {
            return null;
        }

        //
        // Return connectors for this endpoint, or empty list if no connector
        // is available.
        //
        public override void ConnectorsAsync(Ice.EndpointSelectionType endSel, IEndpointConnectors callback)
        {
            callback.Connectors(new List<IConnector>());
        }

        //
        // Return an acceptor for this endpoint, or null if no acceptors
        // is available.
        //
        public override IAcceptor? Acceptor(string adapterName)
        {
            return null;
        }

        //
        // Expand endpoint out in to separate endpoints for each local
        // host if listening on INADDR_ANY on server side or if no host
        // was specified on client side.
        //
        public override List<Endpoint> ExpandIfWildcard()
        {
            return new List<Endpoint> { this };
        }

        public override List<Endpoint> ExpandHost(out Endpoint? publishedEndpoint)
        {
            publishedEndpoint = null;
            return new List<Endpoint>() { this };
        }

        //
        // Check whether the endpoint is equivalent to another one.
        //
        public override bool Equivalent(Endpoint endpoint)
        {
            return false;
        }

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
            if (_type > -1)
            {
                s += " -t " + _type;
            }
            s += " -e " + Ice.Util.EncodingToString(_rawEncoding);
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

            OpaqueEndpointI p = (OpaqueEndpointI)obj;
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
                        if (_type > -1)
                        {
                            throw new FormatException($"multiple -t options in endpoint {endpoint}");
                        }

                        if (argument == null)
                        {
                            throw new FormatException($"no argument provided for -t option in endpoint {endpoint}");
                        }

                        int t;
                        try
                        {
                            t = int.Parse(argument, CultureInfo.InvariantCulture);
                        }
                        catch (FormatException ex)
                        {
                            throw new FormatException($"invalid type value `{argument}' in endpoint {endpoint}", ex);
                        }

                        if (t < 0 || t > 65535)
                        {
                            throw new FormatException($"type value `{argument}' out of range in endpoint {endpoint}");
                        }

                        _type = (short)t;
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
                            _rawEncoding = Ice.Util.StringToEncoding(argument);
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

        private short _type;
        private Ice.Encoding _rawEncoding;
        private byte[] _rawBytes;
        private int _hashCode = 0; // 0 is a special value that means not initialized.
    }

}
