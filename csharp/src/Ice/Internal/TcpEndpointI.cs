// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using System.Globalization;
using System.Net;
using System.Net.Security;

namespace Ice.Internal;

internal sealed class TcpEndpointI : IPEndpointI
{
    public TcpEndpointI(
        ProtocolInstance instance,
        string ho,
        int po,
        EndPoint sourceAddr,
        int ti,
        string conId,
        bool co)
        : base(instance, ho, po, sourceAddr, conId)
    {
        _timeout = ti;
        _compress = co;
    }

    public TcpEndpointI(ProtocolInstance instance)
        : base(instance)
    {
        // The default timeout for TCP endpoints is 60,000 milliseconds (1 minute).
        // This timeout is not used in Ice 3.8 or greater.
        _timeout = 60_000;
        _compress = false;
    }

    public TcpEndpointI(ProtocolInstance instance, Ice.InputStream s)
        : base(instance, s)
    {
        _timeout = s.readInt();
        _compress = s.readBool();
    }

    private sealed class InfoI : Ice.TCPEndpointInfo
    {
        public InfoI(IPEndpointI e)
        {
            _endpoint = e;
        }

        public override short type()
        {
            return _endpoint.type();
        }

        public override bool datagram()
        {
            return _endpoint.datagram();
        }

        public override bool secure()
        {
            return _endpoint.secure();
        }

        private IPEndpointI _endpoint;
    }

    public override void streamWriteImpl(Ice.OutputStream s)
    {
        base.streamWriteImpl(s);
        s.writeInt(_timeout);
        s.writeBool(_compress);
    }

    public override Ice.EndpointInfo getInfo()
    {
        InfoI info = new InfoI(this);
        fillEndpointInfo(info);
        return info;
    }

    public override int timeout()
    {
        return _timeout;
    }

    public override EndpointI timeout(int timeout)
    {
        if (timeout == _timeout)
        {
            return this;
        }
        else
        {
            return new TcpEndpointI(instance_, host_, port_, sourceAddr_, timeout, connectionId_, _compress);
        }
    }

    public override bool compress()
    {
        return _compress;
    }

    public override EndpointI compress(bool compress)
    {
        if (compress == _compress)
        {
            return this;
        }
        else
        {
            return new TcpEndpointI(instance_, host_, port_, sourceAddr_, _timeout, connectionId_, compress);
        }
    }

    public override bool datagram()
    {
        return false;
    }

    public override Transceiver transceiver()
    {
        return null;
    }

    public override Acceptor acceptor(string adapterName, SslServerAuthenticationOptions serverAuthenticationOptions)
    {
        Debug.Assert(serverAuthenticationOptions is null);
        return new TcpAcceptor(this, instance_, host_, port_);
    }

    public TcpEndpointI endpoint(TcpAcceptor acceptor)
    {
        int port = acceptor.effectivePort();
        if (port == port_)
        {
            return this;
        }
        else
        {
            return new TcpEndpointI(instance_, host_, port, sourceAddr_, _timeout, connectionId_, _compress);
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

        if (_timeout == -1)
        {
            s += " -t infinite";
        }
        else
        {
            s += " -t " + _timeout;
        }

        if (_compress)
        {
            s += " -z";
        }

        return s;
    }

    public override int CompareTo(EndpointI obj)
    {
        if (!(obj is TcpEndpointI))
        {
            return type() < obj.type() ? -1 : 1;
        }

        TcpEndpointI p = (TcpEndpointI)obj;
        if (this == p)
        {
            return 0;
        }

        if (_timeout < p._timeout)
        {
            return -1;
        }
        else if (p._timeout < _timeout)
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

        return base.CompareTo(p);
    }

    public override int GetHashCode() => HashCode.Combine(base.GetHashCode(), _timeout, _compress);

    public override void fillEndpointInfo(Ice.IPEndpointInfo info)
    {
        base.fillEndpointInfo(info);
        info.timeout = _timeout;
        info.compress = _compress;
    }

    public override EndpointI toPublishedEndpoint(string publishedHost)
    {
        // A server endpoint can't have a source address or connection ID.
        Debug.Assert(sourceAddr_ is null && connectionId_.Length == 0);

        if (publishedHost.Length == 0)
        {
            return this;
        }
        else
        {
            return new TcpEndpointI(instance_, publishedHost, port_, sourceAddr: null, _timeout, conId: "", _compress);
        }
    }

    protected override bool checkOption(string option, string argument, string endpoint)
    {
        if (base.checkOption(option, argument, endpoint))
        {
            return true;
        }

        switch (option[1])
        {
            case 't':
            {
                if (argument == null)
                {
                    throw new ParseException($"no argument provided for -t option in endpoint '{endpoint}'");
                }

                if (argument == "infinite")
                {
                    _timeout = -1;
                }
                else
                {
                    try
                    {
                        _timeout = int.Parse(argument, CultureInfo.InvariantCulture);
                        if (_timeout < 1)
                        {
                            throw new ParseException($"invalid timeout value '{argument}' in endpoint '{endpoint}'");
                        }
                    }
                    catch (System.FormatException ex)
                    {
                        throw new ParseException($"invalid timeout value '{argument}' in endpoint '{endpoint}'", ex);
                    }
                }

                return true;
            }

            case 'z':
            {
                if (argument != null)
                {
                    throw new ParseException($"unexpected argument '{argument}' provided for -z option in endpoint '{endpoint}'");
                }

                _compress = true;

                return true;
            }

            default:
            {
                return false;
            }
        }
    }

    protected override Connector createConnector(EndPoint addr, NetworkProxy proxy)
    {
        return new TcpConnector(instance_, addr, proxy, sourceAddr_, _timeout, connectionId_);
    }

    protected override IPEndpointI createEndpoint(string host, int port, string connectionId)
    {
        return new TcpEndpointI(instance_, host, port, sourceAddr_, _timeout, connectionId, _compress);
    }

    private int _timeout;
    private bool _compress;
}

internal sealed class TcpEndpointFactory : EndpointFactory
{
    internal TcpEndpointFactory(ProtocolInstance instance)
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
        IPEndpointI endpt = new TcpEndpointI(_instance);
        endpt.initWithOptions(args, oaEndpoint);
        return endpt;
    }

    public EndpointI read(Ice.InputStream s)
    {
        return new TcpEndpointI(_instance, s);
    }

    public EndpointFactory clone(ProtocolInstance instance)
    {
        return new TcpEndpointFactory(instance);
    }

    private readonly ProtocolInstance _instance;
}
