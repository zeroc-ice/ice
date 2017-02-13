// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Collections;
    using System.Collections.Generic;
    using System.Globalization;

    sealed class OpaqueEndpointI : EndpointI
    {
        public OpaqueEndpointI(List<string> args)
        {
            _type = -1;
            _rawEncoding = Ice.Util.Encoding_1_0;
            _rawBytes = new byte[0];

            initWithOptions(args);

            if(_type < 0)
            {
                throw new Ice.EndpointParseException("no -t option in endpoint " + ToString());
            }
            if(_rawBytes.Length == 0)
            {
                throw new Ice.EndpointParseException("no -v option in endpoint " + ToString());
            }

            calcHashValue();
        }

        public OpaqueEndpointI(short type, BasicStream s)
        {
            _type = type;
            _rawEncoding = s.getReadEncoding();
            int sz = s.getReadEncapsSize();
            _rawBytes = new byte[sz];
            s.readBlob(_rawBytes);

            calcHashValue();
        }

        //
        // Marshal the endpoint
        //
        public override void streamWrite(BasicStream s)
        {
            s.startWriteEncaps(_rawEncoding, Ice.FormatType.DefaultFormat);
            s.writeBlob(_rawBytes);
            s.endWriteEncaps();
        }

        //
        // Convert the endpoint to its string form
        //
        public override string ice_toString_()
        {
            string val = IceUtilInternal.Base64.encode(_rawBytes);
            return "opaque -t " + _type + " -e " + Ice.Util.encodingVersionToString(_rawEncoding) + " -v " + val;
        }

        private sealed class InfoI : Ice.OpaqueEndpointInfo
        {
            public InfoI(short type, Ice.EncodingVersion rawEncoding, byte[] rawBytes) :
                base(-1, false, rawEncoding, rawBytes)
            {
                _type = type;
            }

            override public short type()
            {
                return _type;
            }

            override public bool datagram()
            {
                return false;
            }

            override public bool secure()
            {
                return false;
            }

            private readonly short _type;
        }

        //
        // Return the endpoint information.
        //
        public override Ice.EndpointInfo getInfo()
        {
            return new InfoI(_type, _rawEncoding, _rawBytes);
        }

        //
        // Return the endpoint type
        //
        public override short type()
        {
            return _type;
        }

        //
        // Return the protocol name;
        //
        public override string protocol()
        {
            return "opaque";
        }

        //
        // Return the timeout for the endpoint in milliseconds. 0 means
        // non-blocking, -1 means no timeout.
        //
        public override int timeout()
        {
            return -1;
        }

        //
        // Return a new endpoint with a different timeout value, provided
        // that timeouts are supported by the endpoint. Otherwise the same
        // endpoint is returned.
        //
        public override EndpointI timeout(int t)
        {
            return this;
        }

        public override string connectionId()
        {
            return "";
        }

        //
        // Return a new endpoint with a different connection id.
        //
        public override EndpointI connectionId(string id)
        {
            return this;
        }

        //
        // Return true if the endpoints support bzip2 compress, or false
        // otherwise.
        //
        public override bool compress()
        {
            return false;
        }

        //
        // Return a new endpoint with a different compression value,
        // provided that compression is supported by the
        // endpoint. Otherwise the same endpoint is returned.
        //
        public override EndpointI compress(bool compress)
        {
            return this;
        }

        //
        // Return true if the endpoint is datagram-based.
        //
        public override bool datagram()
        {
            return false;
        }

        //
        // Return true if the endpoint is secure.
        //
        public override bool secure()
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
        public override Transceiver transceiver()
        {
            return null;
        }

        //
        // Return connectors for this endpoint, or empty list if no connector
        // is available.
        //
        public override void connectors_async(Ice.EndpointSelectionType endSel, EndpointI_connectors callback)
        {
            callback.connectors(new List<Connector>());
        }

        //
        // Return an acceptor for this endpoint, or null if no acceptors
        // is available.
        //
        public override Acceptor acceptor(string adapterName)
        {
            return null;
        }

        //
        // Expand endpoint out in to separate endpoints for each local
        // host if listening on INADDR_ANY on server side or if no host
        // was specified on client side.
        //
        public override List<EndpointI> expand()
        {
            List<EndpointI> endps = new List<EndpointI>();
            endps.Add(this);
            return endps;
        }

        //
        // Check whether the endpoint is equivalent to another one.
        //
        public override bool equivalent(EndpointI endpoint)
        {
            return false;
        }

        public override int GetHashCode()
        {
            return _hashCode;
        }

        public override string options()
        {
            string s = "";
            if(_type > -1)
            {
                s += " -t " + _type;
            }
            s += " -e " + Ice.Util.encodingVersionToString(_rawEncoding);
            if(_rawBytes.Length > 0)
            {
                s += " -v " + IceUtilInternal.Base64.encode(_rawBytes);
            }
            return s;
        }

        //
        // Compare endpoints for sorting purposes
        //
        public override int CompareTo(EndpointI obj)
        {
            if(!(obj is OpaqueEndpointI))
            {
                return type() < obj.type() ? -1 : 1;
            }

            OpaqueEndpointI p = (OpaqueEndpointI)obj;
            if(this == p)
            {
                return 0;
            }

            if(_type < p._type)
            {
                return -1;
            }
            else if(p._type < _type)
            {
                return 1;
            }

            if(_rawEncoding.major < p._rawEncoding.major)
            {
                return -1;
            }
            else if(p._rawEncoding.major < _rawEncoding.major)
            {
                return 1;
            }

            if(_rawEncoding.minor < p._rawEncoding.minor)
            {
                return -1;
            }
            else if(p._rawEncoding.minor < _rawEncoding.minor)
            {
                return 1;
            }

            if(_rawBytes.Length < p._rawBytes.Length)
            {
                return -1;
            }
            else if(p._rawBytes.Length < _rawBytes.Length)
            {
                return 1;
            }
            for(int i = 0; i < _rawBytes.Length; i++)
            {
                if(_rawBytes[i] < p._rawBytes[i])
                {
                    return -1;
                }
                else if(p._rawBytes[i] < _rawBytes[i])
                {
                    return 1;
                }
            }

            return 0;
        }

        protected override bool checkOption(string option, string argument, string endpoint)
        {
            switch(option[1])
            {
            case 't':
            {
                if(_type > -1)
                {
                    throw new Ice.EndpointParseException("multiple -t options in endpoint " + endpoint);
                }
                if(argument == null)
                {
                    throw new Ice.EndpointParseException("no argument provided for -t option in endpoint " + endpoint);
                }

                int t;
                try
                {
                    t = System.Int32.Parse(argument, CultureInfo.InvariantCulture);
                }
                catch(System.FormatException)
                {
                    throw new Ice.EndpointParseException("invalid type value `" + argument + "' in endpoint " +
                                                         endpoint);
                }

                if(t < 0 || t > 65535)
                {
                    throw new Ice.EndpointParseException("type value `" + argument + "' out of range in endpoint " +
                                                         endpoint);
                }

                _type = (short)t;
                return true;
            }

            case 'v':
            {
                if(_rawBytes.Length > 0)
                {
                    throw new Ice.EndpointParseException("multiple -v options in endpoint " + endpoint);
                }
                if(argument == null)
                {
                    throw new Ice.EndpointParseException("no argument provided for -v option in endpoint " + endpoint);
                }

                for(int j = 0; j < argument.Length; ++j)
                {
                    if(!IceUtilInternal.Base64.isBase64(argument[j]))
                    {
                        throw new Ice.EndpointParseException("invalid base64 character `" + argument[j] +
                                                             "' (ordinal " + ((int)argument[j]) +
                                                             ") in endpoint " + endpoint);
                    }
                }
                _rawBytes = IceUtilInternal.Base64.decode(argument);
                return true;
            }

            case 'e':
            {
                if(argument == null)
                {
                    throw new Ice.EndpointParseException("no argument provided for -e option in endpoint " + endpoint);
                }

                try
                {
                    _rawEncoding = Ice.Util.stringToEncodingVersion(argument);
                }
                catch(Ice.VersionParseException e)
                {
                    throw new Ice.EndpointParseException("invalid encoding version `" + argument +
                                                         "' in endpoint " + endpoint + ":\n" + e.str);
                }
                return true;
            }

            default:
            {
                return false;
            }
            }
        }

        private void calcHashValue()
        {
            int h = 5381;
            IceInternal.HashUtil.hashAdd(ref h, _type);
            IceInternal.HashUtil.hashAdd(ref h, _rawEncoding);
            IceInternal.HashUtil.hashAdd(ref h, _rawBytes);
            _hashCode = h;
        }

        private short _type;
        private Ice.EncodingVersion _rawEncoding;
        private byte[] _rawBytes;
        private int _hashCode;
    }

}
