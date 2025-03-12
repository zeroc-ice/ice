// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using System.Globalization;
using System.Net;
using System.Net.Security;

namespace Ice.Internal;

internal sealed class UdpEndpointI : IPEndpointI
{
    public UdpEndpointI(
        ProtocolInstance instance,
        string host,
        int port,
        EndPoint sourceAddr,
        string mcastInterface,
        int mttl,
        string connectionId,
        bool compress)
        : base(instance, host, port, sourceAddr, connectionId)
    {
        _mcastInterface = mcastInterface;
        _mcastTtl = mttl;
        _compress = compress;
    }

    public UdpEndpointI(ProtocolInstance instance)
        : base(instance)
    {
    }

    public UdpEndpointI(ProtocolInstance instance, Ice.InputStream s)
        : base(instance, s)
    {
        if (s.getEncoding().Equals(Ice.Util.Encoding_1_0))
        {
            s.readByte();
            s.readByte();
            s.readByte();
            s.readByte();
        }
        _compress = s.readBool();
    }

    //
    // Return the endpoint information.
    //
    public override EndpointInfo getInfo()
    {
        Debug.Assert(!secure());
        Debug.Assert(type() == UDPEndpointType.value);

        return new UDPEndpointInfo(
            _compress,
            host_,
            port_,
            Network.endpointAddressToString(sourceAddr_),
            _mcastInterface,
            _mcastTtl);
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
    public override EndpointI timeout(int timeout)
    {
        return this;
    }

    //
    // Return true if the endpoints support bzip2 compress, or false
    // otherwise.
    //
    public override bool compress()
    {
        return _compress;
    }

    //
    // Return a new endpoint with a different compression value,
    // provided that compression is supported by the
    // endpoint. Otherwise the same endpoint is returned.
    //
    public override EndpointI compress(bool compress)
    {
        if (compress == _compress)
        {
            return this;
        }
        else
        {
            return new UdpEndpointI(
                instance_,
                host_,
                port_,
                sourceAddr_,
                _mcastInterface,
                _mcastTtl,
                connectionId_,
                compress);
        }
    }

    //
    // Return true if the endpoint is datagram-based.
    //
    public override bool datagram()
    {
        return true;
    }

    //
    // Return a server side transceiver for this endpoint, or null if a
    // transceiver can only be created by an acceptor.
    //
    public override Transceiver transceiver()
    {
        return new UdpTransceiver(this, instance_, host_, port_, _mcastInterface);
    }

    //
    // Return an acceptor for this endpoint, or null if no acceptors
    // is available.
    //
    public override Acceptor acceptor(string adapterName, SslServerAuthenticationOptions serverAuthenticationOptions)
    {
        Debug.Assert(serverAuthenticationOptions is null);
        return null;
    }

    public override void initWithOptions(List<string> args, bool oaEndpoint)
    {
        base.initWithOptions(args, oaEndpoint);

        if (_mcastInterface == "*")
        {
            if (oaEndpoint)
            {
                _mcastInterface = "";
            }
            else
            {
                throw new ParseException($"'--interface *' not valid for proxy endpoint '{this}'");
            }
        }
    }

    public UdpEndpointI endpoint(UdpTransceiver transceiver)
    {
        int port = transceiver.effectivePort();
        if (port == port_)
        {
            return this;
        }
        else
        {
            return new UdpEndpointI(
                instance_,
                host_,
                port,
                sourceAddr_,
                _mcastInterface,
                _mcastTtl,
                connectionId_,
                _compress);
        }
    }

    public override string options()
    {
        //
        // WARNING: Certain features, such as proxy validation in Glacier2,
        // depend on the format of proxy strings. Changes to toString() and
        // methods called to generate parts of the reference string could break
        // these features. Please review for all features that depend on the
        // format of proxyToString() before changing this and related code.
        //
        string s = base.options();

        if (_mcastInterface.Length != 0)
        {
            bool addQuote = _mcastInterface.Contains(':', StringComparison.Ordinal);
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

        if (_compress)
        {
            s += " -z";
        }

        return s;
    }

    //
    // Compare endpoints for sorting purposes
    //
    public override int CompareTo(EndpointI obj)
    {
        if (!(obj is UdpEndpointI))
        {
            return type() < obj.type() ? -1 : 1;
        }

        UdpEndpointI p = (UdpEndpointI)obj;
        if (this == p)
        {
            return 0;
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
    public override void streamWriteImpl(Ice.OutputStream s)
    {
        base.streamWriteImpl(s);
        if (s.getEncoding().Equals(Ice.Util.Encoding_1_0))
        {
            ProtocolVersion.ice_write(s, Ice.Util.Protocol_1_0);
            EncodingVersion.ice_write(s, Ice.Util.Encoding_1_0);
        }
        s.writeBool(_compress);
    }

    public override int GetHashCode() => HashCode.Combine(base.GetHashCode(), _mcastInterface, _mcastTtl, _compress);

    public override EndpointI toPublishedEndpoint(string publishedHost) =>
        new UdpEndpointI(
            instance_,
            publishedHost.Length > 0 ? publishedHost : host_,
            port_,
            sourceAddr: null,
            mcastInterface: "",
            mttl: -1,
            connectionId: "",
            _compress);

    protected override bool checkOption(string option, string argument, string endpoint)
    {
        if (base.checkOption(option, argument, endpoint))
        {
            return true;
        }

        if (option == "-z")
        {
            if (argument != null)
            {
                throw new ParseException($"unexpected argument '{argument}' provided for -z option in endpoint '{endpoint}'");
            }

            _compress = true;
        }
        else if (option == "-v" || option == "-e")
        {
            if (argument == null)
            {
                throw new ParseException($"no argument provided for {option} option in endpoint '{endpoint}'");
            }

            try
            {
                Ice.EncodingVersion v = Ice.Util.stringToEncodingVersion(argument);
                if (v.major != 1 || v.minor != 0)
                {
                    instance_.logger().warning("deprecated udp endpoint option: " + option);
                }
            }
            catch (ParseException ex)
            {
                throw new ParseException($"invalid version '{argument}' in endpoint '{endpoint}'", ex);
            }
        }
        else if (option == "--ttl")
        {
            if (argument == null)
            {
                throw new ParseException($"no argument provided for --ttl option in endpoint '{endpoint}'");
            }

            try
            {
                _mcastTtl = int.Parse(argument, CultureInfo.InvariantCulture);
            }
            catch (FormatException ex)
            {
                throw new ParseException($"invalid TTL value '{argument}' in endpoint '{endpoint}'", ex);
            }

            if (_mcastTtl < 0)
            {
                throw new ParseException($"TTL value '{argument}' out of range in endpoint '{endpoint}'");
            }
        }
        else if (option == "--interface")
        {
            if (argument == null)
            {
                throw new ParseException($"no argument provided for --interface option in endpoint '{endpoint}'");
            }
            _mcastInterface = argument;
        }
        else
        {
            return false;
        }

        return true;
    }

    protected override Connector createConnector(EndPoint addr, NetworkProxy proxy)
    {
        return new UdpConnector(instance_, addr, sourceAddr_, _mcastInterface, _mcastTtl, connectionId_);
    }

    protected override IPEndpointI createEndpoint(string host, int port, string connectionId)
    {
        return new UdpEndpointI(
            instance_,
            host,
            port,
            sourceAddr_,
            _mcastInterface,
            _mcastTtl,
            connectionId,
            _compress);
    }

    private string _mcastInterface = "";
    private int _mcastTtl = -1;
    private bool _compress;
}

internal sealed class UdpEndpointFactory : EndpointFactory
{
    internal UdpEndpointFactory(ProtocolInstance instance)
    {
        _instance = instance;
    }

    public void initialize()
    {
    }

    public short type()
    {
        return _instance.type();
    }

    public string protocol()
    {
        return _instance.protocol();
    }

    public EndpointI create(List<string> args, bool oaEndpoint)
    {
        IPEndpointI endpt = new UdpEndpointI(_instance);
        endpt.initWithOptions(args, oaEndpoint);
        return endpt;
    }

    public EndpointI read(Ice.InputStream s)
    {
        return new UdpEndpointI(_instance, s);
    }

    public EndpointFactory clone(ProtocolInstance instance)
    {
        return new UdpEndpointFactory(instance);
    }

    private readonly ProtocolInstance _instance;
}
