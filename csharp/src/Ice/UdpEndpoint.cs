//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Net;

namespace IceInternal
{
    internal sealed class UdpEndpoint : IPEndpoint
    {
        public UdpEndpoint(TransportInstance instance, string ho, int po, EndPoint? sourceAddr, string mcastInterface,
                            int mttl, bool conn, string conId, bool co) :
            base(instance, ho, po, sourceAddr, conId)
        {
            _mcastInterface = mcastInterface;
            _mcastTtl = mttl;
            _connect = conn;
            _compress = co;
        }

        public UdpEndpoint(TransportInstance instance) :
            base(instance)
        {
            _connect = false;
            _compress = false;
        }

        public UdpEndpoint(TransportInstance instance, Ice.InputStream s) :
            base(instance, s)
        {
            if (s.Encoding.Equals(Encoding.V1_0))
            {
                s.ReadByte();
                s.ReadByte();
                s.ReadByte();
                s.ReadByte();
            }
            // Not transmitted.
            //_connect = s.readBool();
            _connect = false;
            _compress = s.ReadBool();
        }

        private sealed class InfoI : Ice.UDPEndpointInfo
        {
            public InfoI(UdpEndpoint e) => _endpoint = e;

            public override short Type() => _endpoint.Type();

            public override bool Datagram() => _endpoint.Datagram();

            public override bool Secure() => _endpoint.Secure();

            private readonly UdpEndpoint _endpoint;
        }

        //
        // Return the endpoint information.
        //
        public override Ice.EndpointInfo GetInfo()
        {
            var info = new InfoI(this);
            FillEndpointInfo(info);
            return info;
        }

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
        public override Endpoint Timeout(int timeout) => this;

        //
        // Return true if the endpoints support bzip2 compress, or false
        // otherwise.
        //
        public override bool Compress() => _compress;

        //
        // Return a new endpoint with a different compression value,
        // provided that compression is supported by the
        // endpoint. Otherwise the same endpoint is returned.
        //
        public override Endpoint Compress(bool compress)
        {
            if (compress == _compress)
            {
                return this;
            }
            else
            {
                return new UdpEndpoint(Instance, Host!, Port, SourceAddr, _mcastInterface, _mcastTtl, _connect,
                                        ConnectionId_, compress);
            }
        }

        //
        // Return true if the endpoint is datagram-based.
        //
        public override bool Datagram() => true;

        //
        // Return a server side transceiver for this endpoint, or null if a
        // transceiver can only be created by an acceptor.
        //
        public override ITransceiver Transceiver() =>
            new UdpTransceiver(this, Instance, Host!, Port, _mcastInterface, _connect);

        //
        // Return an acceptor for this endpoint, or null if no acceptors
        // is available.
        //
        public override IAcceptor? Acceptor(string adapterName) => null;

        public override void InitWithOptions(List<string> args, bool oaEndpoint)
        {
            base.InitWithOptions(args, oaEndpoint);

            if (_mcastInterface.Equals("*"))
            {
                if (oaEndpoint)
                {
                    _mcastInterface = "";
                }
                else
                {
                    throw new FormatException($"`--interface *' not valid for proxy endpoint `{this}'");
                }
            }
        }

        public UdpEndpoint Endpoint(UdpTransceiver transceiver)
        {
            int port = transceiver.EffectivePort();
            if (port == Port)
            {
                return this;
            }
            else
            {
                return new UdpEndpoint(Instance, Host!, port, SourceAddr, _mcastInterface, _mcastTtl, _connect,
                                       ConnectionId_, _compress);
            }
        }

        public override string Options()
        {
            //
            // WARNING: Certain features, such as proxy validation in Glacier2,
            // depend on the format of proxy strings. Changes to toString() and
            // methods called to generate parts of the reference string could break
            // these features. Please review for all features that depend on the
            // format of proxyToString() before changing this and related code.
            //
            string s = base.Options();

            if (_mcastInterface.Length != 0)
            {
                bool addQuote = _mcastInterface.IndexOf(':') != -1;
                s += " --interface ";
                if (addQuote)
                {
                    s += "\"";
                }
                s += _mcastInterface;
                if (addQuote)
                {
                    s += "\"";
                }
            }

            if (_mcastTtl != -1)
            {
                s += " --ttl " + _mcastTtl;
            }

            if (_connect)
            {
                s += " -c";
            }

            if (_compress)
            {
                s += " -z";
            }

            return s;
        }

        //
        // Compare endpoints for sorting purposes
        //
        public override int CompareTo(Endpoint obj)
        {
            if (!(obj is UdpEndpoint))
            {
                return Type() < obj.Type() ? -1 : 1;
            }

            var p = (UdpEndpoint)obj;
            if (this == p)
            {
                return 0;
            }

            if (!_connect && p._connect)
            {
                return -1;
            }
            else if (!p._connect && _connect)
            {
                return 1;
            }

            if (!_compress && p._compress)
            {
                return -1;
            }
            else if (!p._compress && _compress)
            {
                return 1;
            }

            int rc = string.Compare(_mcastInterface, p._mcastInterface, StringComparison.Ordinal);
            if (rc != 0)
            {
                return rc;
            }

            if (_mcastTtl < p._mcastTtl)
            {
                return -1;
            }
            else if (p._mcastTtl < _mcastTtl)
            {
                return 1;
            }

            return base.CompareTo(p);
        }

        //
        // Marshal the endpoint
        //
        public override void StreamWriteImpl(Ice.OutputStream s)
        {
            base.StreamWriteImpl(s);
            if (s.Encoding.Equals(Encoding.V1_0))
            {
                s.WriteByte((byte)Protocol.Ice1);
                s.WriteByte(0);
                s.WriteByte(Encoding.V1_0.Major);
                s.WriteByte(Encoding.V1_0.Minor);
            }
            // Not transmitted.
            //s.writeBool(_connect);
            s.WriteBool(_compress);
        }

        public override void HashInit(ref HashCode hash)
        {
            base.HashInit(ref hash);
            hash.Add(_mcastInterface);
            hash.Add(_mcastTtl);
            hash.Add(_connect);
            hash.Add(_compress);
        }

        public override void FillEndpointInfo(Ice.IPEndpointInfo info)
        {
            base.FillEndpointInfo(info);
            if (info is Ice.UDPEndpointInfo udpInfo)
            {
                udpInfo.Timeout = -1;
                udpInfo.Compress = _compress;
                udpInfo.McastInterface = _mcastInterface;
                udpInfo.McastTtl = _mcastTtl;
            }
        }

        protected override bool CheckOption(string option, string? argument, string endpoint)
        {
            if (base.CheckOption(option, argument, endpoint))
            {
                return true;
            }

            if (option.Equals("-c"))
            {
                if (argument != null)
                {
                    throw new FormatException($"unexpected argument `{argument} ' provided for -c option in {endpoint}");
                }

                _connect = true;
            }
            else if (option.Equals("-z"))
            {
                if (argument != null)
                {
                    throw new FormatException($"unexpected argument `{argument}' provided for -z option in {endpoint}");
                }

                _compress = true;
            }
            else if (option.Equals("-v") || option.Equals("-e"))
            {
                if (argument == null)
                {
                    throw new FormatException($"no argument provided for {option} option in endpoint {endpoint}");
                }

                try
                {
                    Ice.Encoding v = Encoding.Parse(argument);
                    if (v.Major != 1 || v.Minor != 0)
                    {
                        Instance.Logger.Warning($"deprecated udp endpoint option: {option}");
                    }
                }
                catch (FormatException ex)
                {
                    throw new FormatException("invalid version `{argument}' in endpoint {endpoint}", ex);
                }
            }
            else if (option.Equals("--ttl"))
            {
                if (argument == null)
                {
                    throw new FormatException($"no argument provided for --ttl option in endpoint {endpoint}");
                }

                try
                {
                    _mcastTtl = int.Parse(argument, CultureInfo.InvariantCulture);
                }
                catch (FormatException ex)
                {
                    throw new FormatException($"invalid TTL value `{argument}' in endpoint {endpoint}", ex);
                }

                if (_mcastTtl < 0)
                {
                    throw new FormatException("TTL value `{argument}' out of range in endpoint {endpoint}");
                }
            }
            else if (option.Equals("--interface"))
            {
                _mcastInterface = argument ??
                    throw new FormatException("no argument provided for --interface option in endpoint {endpoint}");
            }
            else
            {
                return false;
            }

            return true;
        }

        protected override IConnector CreateConnector(EndPoint addr, INetworkProxy? proxy) =>
            new UdpConnector(Instance, addr, SourceAddr, _mcastInterface, _mcastTtl, ConnectionId_);

        protected override IPEndpoint CreateEndpoint(string? host, int port, string connectionId) =>
            new UdpEndpoint(Instance, host!, port, SourceAddr, _mcastInterface, _mcastTtl, _connect, connectionId,
                             _compress);

        private string _mcastInterface = "";
        private int _mcastTtl = -1;
        private bool _connect;
        private bool _compress;
    }

    internal sealed class UdpEndpointFactory : IEndpointFactory
    {
        internal UdpEndpointFactory(TransportInstance instance) => _instance = instance;

        public void Initialize()
        {
        }

        public short Type() => _instance!.Type;

        public string Transport() => _instance!.Transport;

        public Endpoint Create(List<string> args, bool oaEndpoint)
        {
            IPEndpoint endpt = new UdpEndpoint(_instance!);
            endpt.InitWithOptions(args, oaEndpoint);
            return endpt;
        }

        public Endpoint Read(Ice.InputStream s) => new UdpEndpoint(_instance!, s);

        public void Destroy() => _instance = null;

        public IEndpointFactory Clone(TransportInstance instance) => new UdpEndpointFactory(instance);

        private TransportInstance? _instance;
    }

}
