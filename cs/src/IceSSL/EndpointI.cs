// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceSSL
{
    using System.Diagnostics;
    using System.Collections;

    sealed class EndpointI : IceInternal.EndpointI
    {
        internal const short TYPE = 2;

        internal EndpointI(Instance instance, string ho, int po, int ti, string conId, bool co, bool oae)
        {
            instance_ = instance;
            host_ = ho;
            port_ = po;
            timeout_ = ti;
            connectionId_ = conId;
            compress_ = co;
            oaEndpoint_ = oae;
            calcHashValue();
        }

        internal EndpointI(Instance instance, string str, bool oaEndpoint)
        {
            instance_ = instance;
            host_ = null;
            port_ = 0;
            timeout_ = -1;
            compress_ = false;
            oaEndpoint_ = oaEndpoint;

            char[] separators = { ' ', '\t', '\n', '\r' };
            string[] arr = str.Split(separators);

            int i = 0;
            while(i < arr.Length)
            {
                if(arr[i].Length == 0)
                {
                    i++;
                    continue;
                }

                string option = arr[i++];
                if(option.Length != 2 || option[0] != '-')
                {
                    Ice.EndpointParseException e = new Ice.EndpointParseException();
                    e.str = "ssl " + str;
                    throw e;
                }

                string argument = null;
                if(i < arr.Length && arr[i][0] != '-')
                {
                    argument = arr[i++];
                }

                switch(option[1])
                {
                    case 'h': 
                    {
                        if(argument == null)
                        {
                            Ice.EndpointParseException e = new Ice.EndpointParseException();
                            e.str = "ssl " + str;
                            throw e;
                        }

                        host_ = argument;
                        break;
                    }

                    case 'p': 
                    {
                        if(argument == null)
                        {
                            Ice.EndpointParseException e = new Ice.EndpointParseException();
                            e.str = "ssl " + str;
                            throw e;
                        }

                        try
                        {
                            port_ = System.Int32.Parse(argument);
                        }
                        catch(System.FormatException ex)
                        {
                            Ice.EndpointParseException e = new Ice.EndpointParseException(ex);
                            e.str = "ssl " + str;
                            throw e;
                        }

                        if(port_ < 0 || port_ > 65535)
                        {
                            Ice.EndpointParseException e = new Ice.EndpointParseException();
                            e.str = "ssl " + str;
                            throw e;
                        }

                        break;
                    }

                    case 't': 
                    {
                        if(argument == null)
                        {
                            Ice.EndpointParseException e = new Ice.EndpointParseException();
                            e.str = "ssl " + str;
                            throw e;
                        }

                        try
                        {
                            timeout_ = System.Int32.Parse(argument);
                        }
                        catch(System.FormatException ex)
                        {
                            Ice.EndpointParseException e = new Ice.EndpointParseException(ex);
                            e.str = "ssl " + str;
                            throw e;
                        }

                        break;
                    }

                    case 'z': 
                    {
                        if(argument != null)
                        {
                            Ice.EndpointParseException e = new Ice.EndpointParseException();
                            e.str = "ssl " + str;
                            throw e;
                        }

                        compress_ = true;
                        break;
                    }

                    default: 
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
                        e.str = "ssl " + str;
                        throw e;
                    }
                }
            }

            if(host_ == null)
            {
                host_ = instance_.defaultHost();
                if(host_ == null)
                {
                    if(oaEndpoint_)
                    {
                        host_ = "0.0.0.0";
                    }
                    else
                    {
                        host_ = "127.0.0.1";
                    }
                }
            }
            else if(host_.Equals("*"))
            {
                host_ = "0.0.0.0";
            }
            calcHashValue();
        }

        internal EndpointI(Instance instance, IceInternal.BasicStream s)
        {
            instance_ = instance;
            s.startReadEncaps();
            host_ = s.readString();
            port_ = s.readInt();
            timeout_ = s.readInt();
            compress_ = s.readBool();
            s.endReadEncaps();
            oaEndpoint_ = false;
            calcHashValue();
        }

        //
        // Marshal the endpoint.
        //
        public override void streamWrite(IceInternal.BasicStream s)
        {
            s.writeShort(TYPE);
            s.startWriteEncaps();
            s.writeString(host_);
            s.writeInt(port_);
            s.writeInt(timeout_);
            s.writeBool(compress_);
            s.endWriteEncaps();
        }

        //
        // Convert the endpoint to its string form.
        //
        public override string ice_toString_()
        {
            //
            // WARNING: Certain features, such as proxy validation in Glacier2,
            // depend on the format of proxy strings. Changes to toString() and
            // methods called to generate parts of the reference string could break
            // these features. Please review for all features that depend on the
            // format of proxyToString() before changing this and related code.
            //
            string s = "ssl -h " + host_ + " -p " + port_;
            if(timeout_ != -1)
            {
                s += " -t " + timeout_;
            }
            if(compress_)
            {
                s += " -z";
            }
            return s;
        }

        //
        // Return the endpoint type.
        //
        public override short type()
        {
            return TYPE;
        }

        //
        // Return the timeout for the endpoint in milliseconds. 0 means
        // non-blocking, -1 means no timeout.
        //
        public override int timeout()
        {
            return timeout_;
        }

        //
        // Return a new endpoint with a different timeout value, provided
        // that timeouts are supported by the endpoint. Otherwise the same
        // endpoint is returned.
        //
        public override IceInternal.EndpointI timeout(int timeout)
        {
            if(timeout == timeout_)
            {
                return this;
            }
            else
            {
                return new EndpointI(instance_, host_, port_, timeout, connectionId_, compress_, oaEndpoint_);
            }
        }

        //
        // Return a new endpoint with a different connection id.
        //
        public override IceInternal.EndpointI connectionId(string connectionId)
        {
            if(connectionId == connectionId_)
            {
                return this;
            }
            else
            {
                return new EndpointI(instance_, host_, port_, timeout_, connectionId, compress_, oaEndpoint_);
            }
        }

        //
        // Return true if the endpoints support bzip2 compress, or false
        // otherwise.
        //
        public override bool compress()
        {
            return compress_;
        }

        //
        // Return a new endpoint with a different compression value,
        // provided that compression is supported by the
        // endpoint. Otherwise the same endpoint is returned.
        //
        public override IceInternal.EndpointI compress(bool compress)
        {
            if(compress == compress_)
            {
                return this;
            }
            else
            {
                return new EndpointI(instance_, host_, port_, timeout_, connectionId_, compress, oaEndpoint_);
            }
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
            return true;
        }

        //
        // Return true if the endpoint type is unknown.
        //
        public override bool unknown()
        {
            return false;
        }

        //
        // Return a server side transceiver for this endpoint, or null if a
        // transceiver can only be created by an acceptor. In case a
        // transceiver is created, this operation also returns a new
        // "effective" endpoint, which might differ from this endpoint,
        // for example, if a dynamic port number is assigned.
        //
        public override IceInternal.Transceiver transceiver(ref IceInternal.EndpointI endpoint)
        {
            endpoint = this;
            return null;
        }

        //
        // Return a connector for this endpoint, or empty list if no connector
        // is available.
        //
        public override ArrayList connectors()
        {
            ArrayList connectors = new ArrayList();
            System.Net.IPEndPoint[] addresses = IceInternal.Network.getAddresses(host_, port_);
            for(int i = 0; i < addresses.Length; ++i)
            {
                connectors.Add(new ConnectorI(instance_, addresses[i], timeout_, connectionId_));
            }
            return connectors;
        }

        //
        // Return an acceptor for this endpoint, or null if no acceptor
        // is available. In case an acceptor is created, this operation
        // also returns a new "effective" endpoint, which might differ
        // from this endpoint, for example, if a dynamic port number is
        // assigned.
        //
        public override IceInternal.Acceptor acceptor(ref IceInternal.EndpointI endpoint, string adapterName)
        {
            AcceptorI p = new AcceptorI(instance_, adapterName, host_, port_);
            endpoint = 
                new EndpointI(instance_, host_, p.effectivePort(), timeout_, connectionId_, compress_, oaEndpoint_);
            return p;
        }

        //
        // Expand endpoint out in to separate endpoints for each local
        // host if endpoint was configured with no host set.
        //
        public override ArrayList expand()
        {
            ArrayList endps = new ArrayList();
            if(host_.Equals("0.0.0.0"))
            {
                string[] hosts = IceInternal.Network.getLocalHosts();
                for(int i = 0; i < hosts.Length; ++i)
                {
                    if(!oaEndpoint_ || hosts.Length == 1 || !hosts[i].Equals("127.0.0.1"))
                    {
                        endps.Add(new EndpointI(instance_, hosts[i], port_, timeout_, connectionId_, compress_,
                                                oaEndpoint_));
                    }
                }
            }
            else
            {
                endps.Add(this);
            }
            return endps;
        }

        //
        // Check whether the endpoint is equivalent to a specific
        // Transceiver or Acceptor
        //
        public override bool equivalent(IceInternal.Transceiver transceiver)
        {
            return false;
        }

        public override bool equivalent(IceInternal.Acceptor acceptor)
        {
            AcceptorI sslAcceptor = null;
            try
            {
                sslAcceptor = (AcceptorI)acceptor;
            }
            catch(System.InvalidCastException)
            {
                return false;
            }
            return sslAcceptor.equivalent(host_, port_);
        }

        public override bool requiresThreadPerConnection()
        {
            return true;
        }

        public override int GetHashCode()
        {
            return hashCode_;
        }

        //
        // Compare endpoints for sorting purposes
        //
        public override bool Equals(object obj)
        {
            return CompareTo(obj) == 0;
        }

        public override int CompareTo(object obj)
        {
            EndpointI p = null;

            try
            {
                p = (EndpointI)obj;
            }
            catch(System.InvalidCastException)
            {
                try
                {
                    IceInternal.EndpointI e = (IceInternal.EndpointI)obj;
                    return type() < e.type() ? -1 : 1;
                }
                catch(System.InvalidCastException)
                {
                    Debug.Assert(false);
                }
            }

            if(this == p)
            {
                return 0;
            }

            if(port_ < p.port_)
            {
                return -1;
            }
            else if(p.port_ < port_)
            {
                return 1;
            }

            if(timeout_ < p.timeout_)
            {
                return -1;
            }
            else if(p.timeout_ < timeout_)
            {
                return 1;
            }

            if(!connectionId_.Equals(p.connectionId_))
            {
                return connectionId_.CompareTo(p.connectionId_);
            }

            if(!compress_ && p.compress_)
            {
                return -1;
            }
            else if(!p.compress_ && compress_)
            {
                return 1;
            }

            return host_.CompareTo(p.host_);
        }

        private void calcHashValue()
        {
            try
            {
                hashCode_ = IceInternal.Network.getNumericHost(host_).GetHashCode();
            }
            catch(Ice.DNSException)
            {
                hashCode_ = host_.GetHashCode();
            }
            hashCode_ = 5 * hashCode_ + port_;
            hashCode_ = 5 * hashCode_ + timeout_;
            hashCode_ = 5 * hashCode_ + connectionId_.GetHashCode();
            hashCode_ = 5 * hashCode_ + (compress_? 1 : 0);
        }

        private Instance instance_;
        private string host_;
        private int port_;
        private int timeout_;
        private string connectionId_ = "";
        private bool compress_;
        private bool oaEndpoint_;
        private int hashCode_;
    }

    internal sealed class EndpointFactoryI : IceInternal.EndpointFactory
    {
        internal EndpointFactoryI(Instance instance)
        {
            instance_ = instance;
        }

        public short type()
        {
            return EndpointI.TYPE;
        }

        public string protocol()
        {
            return "ssl";
        }

        public IceInternal.EndpointI create(string str, bool oaEndpoint)
        {
            return new EndpointI(instance_, str, oaEndpoint);
        }

        public IceInternal.EndpointI read(IceInternal.BasicStream s)
        {
            return new EndpointI(instance_, s);
        }

        public void destroy()
        {
            instance_ = null;
        }

        private Instance instance_;
    }
}
