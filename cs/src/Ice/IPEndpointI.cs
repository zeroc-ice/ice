// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Globalization;
    using System.Net;
    using System;

    public abstract class IPEndpointI : EndpointI
    {
        public IPEndpointI(ProtocolInstance instance, string host, int port, string connectionId)
        {
            instance_ = instance;
            host_ = host;
            port_ = port;
            connectionId_ = connectionId;
            _hashInitialized = false;
        }

        public IPEndpointI(ProtocolInstance instance)
        {
            instance_ = instance;
            host_ = null;
            port_ = 0;
            connectionId_ = "";
            _hashInitialized = false;
        }

        public IPEndpointI(ProtocolInstance instance, BasicStream s)
        {
            instance_ = instance;
            host_ = s.readString();
            port_ = s.readInt();
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

        public override void streamWrite(BasicStream s)
        {
            s.startWriteEncaps();
            streamWriteImpl(s);
            s.endWriteEncaps();
        }

        public override short type()
        {
            return instance_.type();
        }

        public override string protocol()
        {
            return instance_.protocol();
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

        public override List<Connector> connectors(Ice.EndpointSelectionType selType)
        {
#if SILVERLIGHT
            return connectors(Network.getAddresses(host_,
                                                   port_,
                                                   instance_.protocolSupport(),
                                                   selType,
                                                   instance_.preferIPv6(),
                                                   false),
                              instance_.networkProxy());
#else
            return instance_.resolve(host_, port_, selType, this);
#endif
        }

        public override void connectors_async(Ice.EndpointSelectionType selType, EndpointI_connectors callback)
        {
#if SILVERLIGHT
            callback.connectors(connectors(selType));
#else
            instance_.resolve(host_, port_, selType, this, callback);
#endif
        }

        public override List<EndpointI> expand()
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

        public override bool equivalent(EndpointI endpoint)
        {
            if(!(endpoint is IPEndpointI))
            {
                return false;
            }
            IPEndpointI ipEndpointI = (IPEndpointI)endpoint;
            return ipEndpointI.type() == type() && ipEndpointI.host_.Equals(host_) && ipEndpointI.port_ == port_;
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

            return string.Compare(connectionId_, p.connectionId_, StringComparison.Ordinal);
        }

        public string host()
        {
            return host_;
        }

        public int port()
        {
            return port_;
        }

        protected virtual void streamWriteImpl(BasicStream s)
        {
            s.writeString(host_);
            s.writeInt(port_);
        }

        protected virtual void hashInit(ref int h)
        {
            HashUtil.hashAdd(ref h, host_);
            HashUtil.hashAdd(ref h, port_);
            HashUtil.hashAdd(ref h, connectionId_);
        }

        protected virtual void fillEndpointInfo(Ice.IPEndpointInfo info)
        {
            info.host = host_;
            info.port = port_;
        }

        public void initWithOptions(List<string> args, bool oaEndpoint)
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
        }

        protected override bool checkOption(string option, string argument, string endpoint)
        {
            switch(option[1])
            {
                case 'h':
                {
                    if(argument == null)
                    {
                        throw new Ice.EndpointParseException("no argument provided for -h option in endpoint " +
                                                             endpoint);
                    }
                    host_ = argument;
                    return true;
                }

                case 'p':
                {
                    if(argument == null)
                    {
                        throw new Ice.EndpointParseException("no argument provided for -p option in endpoint " +
                                                             endpoint);
                    }

                    try
                    {
                        port_ = System.Int32.Parse(argument, CultureInfo.InvariantCulture);
                    }
                    catch(System.FormatException ex)
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

                    return true;
                }

                default:
                {
                    return false;
                }
            }
        }

        protected abstract Connector createConnector(EndPoint addr, NetworkProxy proxy);
        protected abstract IPEndpointI createEndpoint(string host, int port, string connectionId);

        protected ProtocolInstance instance_;
        protected string host_;
        protected int port_;
        protected string connectionId_;
        private bool _hashInitialized;
        private int _hashValue;
    }

}
