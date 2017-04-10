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

    using System.Collections.Generic;
    using System.Globalization;
    using System.Net;
    using System;

    public abstract class IPEndpointI : EndpointI
    {
        public IPEndpointI(ProtocolInstance instance, string host, int port, EndPoint sourceAddr, string connectionId)
        {
            instance_ = instance;
            host_ = host;
            port_ = port;
            sourceAddr_ = sourceAddr;
            connectionId_ = connectionId;
            _hashInitialized = false;
        }

        public IPEndpointI(ProtocolInstance instance)
        {
            instance_ = instance;
            host_ = null;
            port_ = 0;
            sourceAddr_ = null;
            connectionId_ = "";
            _hashInitialized = false;
        }

        public IPEndpointI(ProtocolInstance instance, Ice.InputStream s)
        {
            instance_ = instance;
            host_ = s.readString();
            port_ = s.readInt();
            sourceAddr_ = null;
            connectionId_ = "";
            _hashInitialized = false;
        }

        private sealed class InfoI : Ice.IPEndpointInfo
        {
            public InfoI(IPEndpointI e)
            {
                _endpoint = e;
            }

            override public short type()
            {
                return _endpoint.type();
            }

            override public bool datagram()
            {
                return _endpoint.datagram();;
            }

            override public bool secure()
            {
                return _endpoint.secure();
            }

            private IPEndpointI _endpoint;
        }

        public override Ice.EndpointInfo getInfo()
        {
            InfoI info = new InfoI(this);
            fillEndpointInfo(info);
            return info;
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
            if(connectionId.Equals(connectionId_))
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

        public override List<EndpointI> expandIfWildcard()
        {
            List<EndpointI> endps = new List<EndpointI>();
            List<string> hosts = Network.getHostsForEndpointExpand(host_, instance_.protocolSupport(), false);
            if(hosts == null || hosts.Count == 0)
            {
                endps.Add(this);
            }
            else
            {
                foreach(string h in hosts)
                {
                    endps.Add(createEndpoint(h, port_, connectionId_));
                }
            }
            return endps;
        }

        public override List<EndpointI> expandHost(out EndpointI publish)
        {
            //
            // If this endpoint has an empty host (wildcard address), don't expand, just return
            // this endpoint.
            //
            var endpoints = new List<EndpointI>();
            if(host_.Length == 0)
            {
                publish = null;
                endpoints.Add(this);
                return endpoints;
            }

            //
            // If using a fixed port, this endpoint can be used as the published endpoint to
            // access the returned endpoints. Otherwise, we'll publish each individual expanded
            // endpoint.
            //
            publish = port_ > 0 ? this : null;

            List<EndPoint> addresses = Network.getAddresses(host_,
                                                            port_,
                                                            instance_.protocolSupport(),
                                                            Ice.EndpointSelectionType.Ordered,
                                                            instance_.preferIPv6(),
                                                            true);

            if(addresses.Count == 1)
            {
                endpoints.Add(this);
            }
            else
            {
                foreach(EndPoint addr in addresses)
                {
                    endpoints.Add(createEndpoint(Network.endpointAddressToString(addr),
                                                 Network.endpointPort(addr),
                                                 connectionId_));
                }
            }
            return endpoints;
        }

        public override bool equivalent(EndpointI endpoint)
        {
            if(!(endpoint is IPEndpointI))
            {
                return false;
            }
            IPEndpointI ipEndpointI = (IPEndpointI)endpoint;
            return ipEndpointI.type() == type() && ipEndpointI.host_.Equals(host_) && ipEndpointI.port_ == port_ &&
                Network.addressEquals(ipEndpointI.sourceAddr_, sourceAddr_);
        }

        public virtual List<Connector> connectors(List<EndPoint> addresses, NetworkProxy proxy)
        {
            List<Connector> connectors = new List<Connector>();
            foreach(EndPoint p in addresses)
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

            if(host_ != null && host_.Length > 0)
            {
                s += " -h ";
                bool addQuote = host_.IndexOf(':') != -1;
                if(addQuote)
                {
                    s += "\"";
                }
                s += host_;
                if(addQuote)
                {
                    s += "\"";
                }
            }

            s += " -p " + port_;

            if(sourceAddr_ != null)
            {
                s += " --sourceAddress " + Network.endpointAddressToString(sourceAddr_);
            }

            return s;
        }

        public override int GetHashCode()
        {
            if(!_hashInitialized)
            {
                _hashValue = 5381;
                HashUtil.hashAdd(ref _hashValue, type());
                hashInit(ref _hashValue);
                _hashInitialized = true;
            }
            return _hashValue;
        }

        public override int CompareTo(EndpointI obj)
        {
            if(!(obj is IPEndpointI))
            {
                return type() < obj.type() ? -1 : 1;
            }

            IPEndpointI p = (IPEndpointI)obj;
            if(this == p)
            {
                return 0;
            }

            int v = string.Compare(host_, p.host_, StringComparison.Ordinal);
            if(v != 0)
            {
                return v;
            }

            if(port_ < p.port_)
            {
                return -1;
            }
            else if(p.port_ < port_)
            {
                return 1;
            }

            int rc = string.Compare(Network.endpointAddressToString(sourceAddr_),
                                    Network.endpointAddressToString(p.sourceAddr_), StringComparison.Ordinal);
            if(rc != 0)
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

        public virtual void hashInit(ref int h)
        {
            HashUtil.hashAdd(ref h, host_);
            HashUtil.hashAdd(ref h, port_);
            if(sourceAddr_ != null)
            {
                HashUtil.hashAdd(ref h, sourceAddr_);
            }
            HashUtil.hashAdd(ref h, connectionId_);
        }

        public virtual void fillEndpointInfo(Ice.IPEndpointInfo info)
        {
            info.host = host_;
            info.port = port_;
            info.sourceAddress = Network.endpointAddressToString(sourceAddr_);
        }

        public virtual void initWithOptions(List<string> args, bool oaEndpoint)
        {
            base.initWithOptions(args);

            if(host_ == null || host_.Length == 0)
            {
                host_ = instance_.defaultHost();
            }
            else if(host_.Equals("*"))
            {
                if(oaEndpoint)
                {
                    host_ = "";
                }
                else
                {
                    throw new Ice.EndpointParseException("`-h *' not valid for proxy endpoint `" + ToString() + "'");
                }
            }

            if(host_ == null)
            {
                host_ = "";
            }

            if(sourceAddr_ != null)
            {
                if(oaEndpoint)
                {
                    throw new Ice.EndpointParseException("`--sourceAddress' not valid for object adapter endpoint `" +
                                                         ToString() + "'");
                }
            }
            else if(!oaEndpoint)
            {
                sourceAddr_ = instance_.defaultSourceAddress();
            }
        }

        protected override bool checkOption(string option, string argument, string endpoint)
        {
            if(option.Equals("-h"))
            {
                if(argument == null)
                {
                    throw new Ice.EndpointParseException("no argument provided for -h option in endpoint " +
                                                         endpoint);
                }
                host_ = argument;
            }
            else if(option.Equals("-p"))
            {
                if(argument == null)
                {
                    throw new Ice.EndpointParseException("no argument provided for -p option in endpoint " +
                                                         endpoint);
                }

                try
                {
                    port_ = int.Parse(argument, CultureInfo.InvariantCulture);
                }
                catch(FormatException ex)
                {
                    Ice.EndpointParseException e = new Ice.EndpointParseException(ex);
                    e.str = "invalid port value `" + argument + "' in endpoint " + endpoint;
                    throw e;
                }

                if(port_ < 0 || port_ > 65535)
                {
                    throw new Ice.EndpointParseException("port value `" + argument +
                                                         "' out of range in endpoint " + endpoint);
                }
            }
            else if(option.Equals("--sourceAddress"))
            {
                if(argument == null)
                {
                    throw new Ice.EndpointParseException("no argument provided for --sourceAddress option in endpoint " +
                                                         endpoint);
                }
                sourceAddr_ = Network.getNumericAddress(argument);
                if(sourceAddr_ == null)
                {
                    throw new Ice.EndpointParseException(
                        "invalid IP address provided for --sourceAddress option in endpoint " + endpoint);
                }
            }
            else
            {
                return false;
            }
            return true;
        }

        protected abstract Connector createConnector(EndPoint addr, NetworkProxy proxy);
        protected abstract IPEndpointI createEndpoint(string host, int port, string connectionId);

        protected ProtocolInstance instance_;
        protected string host_;
        protected int port_;
        protected EndPoint sourceAddr_;
        protected string connectionId_;
        private bool _hashInitialized;
        private int _hashValue;
    }

}
