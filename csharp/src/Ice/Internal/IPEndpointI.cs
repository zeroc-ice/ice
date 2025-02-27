// Copyright (c) ZeroC, Inc.

using System.Globalization;
using System.Net;

namespace Ice.Internal;

public abstract class IPEndpointI : EndpointI
{
    protected IPEndpointI(ProtocolInstance instance, string host, int port, EndPoint sourceAddr, string connectionId)
    {
        instance_ = instance;
        host_ = host;
        _normalizedHost = normalizeHost(host_);
        port_ = port;
        sourceAddr_ = sourceAddr;
        connectionId_ = connectionId;
    }

    protected IPEndpointI(ProtocolInstance instance)
    {
        instance_ = instance;
        host_ = null;
        _normalizedHost = null;
        port_ = 0;
        sourceAddr_ = null;
        connectionId_ = "";
    }

    protected IPEndpointI(ProtocolInstance instance, Ice.InputStream s)
    {
        instance_ = instance;
        host_ = s.readString();
        _normalizedHost = normalizeHost(host_);
        port_ = s.readInt();
        sourceAddr_ = null;
        connectionId_ = "";
    }

    public override short type()
    {
        return instance_.type();
    }

    public override string protocol()
    {
        return instance_.protocol();
    }

    public override bool secure()
    {
        return instance_.secure();
    }

    public override string connectionId()
    {
        return connectionId_;
    }

    public override EndpointI connectionId(string connectionId)
    {
        if (connectionId.Equals(connectionId_, StringComparison.Ordinal))
        {
            return this;
        }
        else
        {
            return createEndpoint(host_, port_, connectionId);
        }
    }

    public override void connectors_async(Ice.EndpointSelectionType selType, EndpointI_connectors callback)
    {
        instance_.resolve(host_, port_, selType, this, callback);
    }

    public override List<EndpointI> expandHost()
    {
        // If this endpoint has an empty host (wildcard address), don't expand, just return this endpoint.
        if (host_.Length == 0)
        {
            return [this];
        }

        List<EndPoint> addresses = Network.getAddresses(
            host_,
            port_,
            instance_.protocolSupport(),
            instance_.preferIPv6(),
            true);

        return addresses.Select(
            addr => createEndpoint(
                        Network.endpointAddressToString(addr),
                        Network.endpointPort(addr),
                        connectionId_) as EndpointI).ToList();
    }

    // Empty host_ means the endpoint is a wildcard address. This method must be called only on an endpoint with an
    // empty host or an IP address.
    public override bool isLoopbackOrMulticast()
    {
        if (host_.Length > 0)
        {
            var ipEndPoint = IPEndPoint.Parse(host_);
            return IPAddress.IsLoopback(ipEndPoint.Address) || Network.isMulticast(ipEndPoint);
        }
        else
        {
            return false;
        }
    }

    public override bool equivalent(EndpointI endpoint)
    {
        if (!(endpoint is IPEndpointI))
        {
            return false;
        }
        IPEndpointI ipEndpointI = (IPEndpointI)endpoint;
        return ipEndpointI.type() == type() &&
            ipEndpointI._normalizedHost == _normalizedHost &&
            ipEndpointI.port_ == port_;
    }

    public virtual List<Connector> connectors(List<EndPoint> addresses, NetworkProxy proxy)
    {
        List<Connector> connectors = new List<Connector>();
        foreach (EndPoint p in addresses)
        {
            connectors.Add(createConnector(p, proxy));
        }
        return connectors;
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
        string s = "";

        if (host_ != null && host_.Length > 0)
        {
            s += " -h ";
            bool addQuote = host_.Contains(':', StringComparison.Ordinal);
            if (addQuote)
            {
                s += "\"";
            }
            s += host_;
            if (addQuote)
            {
                s += "\"";
            }
        }

        s += " -p " + port_;

        if (sourceAddr_ != null)
        {
            string sourceAddr = Network.endpointAddressToString(sourceAddr_);
            bool addQuote = sourceAddr.Contains(':', StringComparison.Ordinal);
            s += " --sourceAddress ";
            if (addQuote)
            {
                s += "\"";
            }
            s += sourceAddr;
            if (addQuote)
            {
                s += "\"";
            }
        }

        return s;
    }

    public override int GetHashCode()
    {
        var hash = new HashCode();
        hash.Add(type());
        hash.Add(host_);
        hash.Add(port_);
        if (sourceAddr_ is not null)
        {
            hash.Add(sourceAddr_);
        }
        hash.Add(connectionId_);
        return hash.ToHashCode();
    }

    public override int CompareTo(EndpointI other)
    {
        if (!(other is IPEndpointI))
        {
            return type() < other.type() ? -1 : 1;
        }

        IPEndpointI p = (IPEndpointI)other;
        if (this == p)
        {
            return 0;
        }

        int v = string.Compare(host_, p.host_, StringComparison.Ordinal);
        if (v != 0)
        {
            return v;
        }

        if (port_ < p.port_)
        {
            return -1;
        }
        else if (p.port_ < port_)
        {
            return 1;
        }

        int rc = string.Compare(
            Network.endpointAddressToString(sourceAddr_),
            Network.endpointAddressToString(p.sourceAddr_),
            StringComparison.Ordinal);
        if (rc != 0)
        {
            return rc;
        }

        return string.Compare(connectionId_, p.connectionId_, StringComparison.Ordinal);
    }

    public override void streamWriteImpl(Ice.OutputStream s)
    {
        s.writeString(host_);
        s.writeInt(port_);
    }

    public virtual void initWithOptions(List<string> args, bool oaEndpoint)
    {
        initWithOptions(args);

        if (host_ == null || host_.Length == 0)
        {
            host_ = instance_.defaultHost();
            _normalizedHost = normalizeHost(host_);
        }
        else if (host_ == "*")
        {
            if (oaEndpoint)
            {
                host_ = "";
                _normalizedHost = "";
            }
            else
            {
                throw new ParseException($"'-h *' not valid for proxy endpoint '{ToString()}'");
            }
        }

        if (host_ == null)
        {
            host_ = "";
            _normalizedHost = normalizeHost(host_);
        }

        if (sourceAddr_ != null)
        {
            if (oaEndpoint)
            {
                throw new ParseException($"'--sourceAddress' not valid for object adapter endpoint '{ToString()}'");
            }
        }
        else if (!oaEndpoint)
        {
            sourceAddr_ = instance_.defaultSourceAddress();
        }
    }

    protected override bool checkOption(string option, string argument, string endpoint)
    {
        if (option == "-h")
        {
            if (argument == null)
            {
                throw new ParseException($"no argument provided for -h option in endpoint '{endpoint}'");
            }
            host_ = argument;
            _normalizedHost = normalizeHost(host_);
        }
        else if (option == "-p")
        {
            if (argument == null)
            {
                throw new ParseException($"no argument provided for -p option in endpoint '{endpoint}'");
            }

            try
            {
                port_ = int.Parse(argument, CultureInfo.InvariantCulture);
            }
            catch (FormatException ex)
            {
                throw new ParseException($"invalid port value '{argument}' in endpoint '{endpoint}'", ex);
            }

            if (port_ < 0 || port_ > 65535)
            {
                throw new ParseException($"port value '{argument}' out of range in endpoint '{endpoint}'");
            }
        }
        else if (option == "--sourceAddress")
        {
            if (argument == null)
            {
                throw new ParseException($"no argument provided for --sourceAddress option in endpoint '{endpoint}'");
            }
            sourceAddr_ = Network.getNumericAddress(argument);
            if (sourceAddr_ == null)
            {
                throw new ParseException(
                    $"invalid IP address provided for --sourceAddress option in endpoint '{endpoint}'");
            }
        }
        else
        {
            return false;
        }
        return true;
    }

    private static string normalizeHost(string host)
    {
        if (host is not null && host.Contains(':', StringComparison.Ordinal))
        {
            // Could be an IPv6 address that we need to normalize.
            try
            {
                return IPAddress.Parse(host).ToString(); // normalized host
            }
            catch
            {
                // Ignore - don't normalize host.
            }
        }
        return host;
    }

    protected abstract Connector createConnector(EndPoint addr, NetworkProxy proxy);

    protected abstract IPEndpointI createEndpoint(string host, int port, string connectionId);

    protected readonly ProtocolInstance instance_;
    protected string host_;
    protected int port_;
    protected EndPoint sourceAddr_;
    protected string connectionId_;
    // Set when we set _host; used by the implementation of equivalent.
    private string _normalizedHost;
}
