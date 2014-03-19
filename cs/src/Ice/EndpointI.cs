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
    using System.Net;
    using System;

    public interface EndpointI_connectors
    {
        void connectors(List<Connector> connectors);
        void exception(Ice.LocalException ex);
    }

    public abstract class EndpointI : Ice.Endpoint, System.IComparable<EndpointI>
    {    
        public EndpointI(string connectionId)
        {
            connectionId_ = connectionId;
        }

        public EndpointI()
        {
        }

        public override string ToString()
        {
            return ice_toString_();
        }

        public abstract string ice_toString_();
        public abstract Ice.EndpointInfo getInfo();

        public override bool Equals(object obj)
        {
            if(!(obj is EndpointI))
            {
                return false;
            }
            return CompareTo((EndpointI)obj) == 0;
        }

        public override int GetHashCode()
        {
            int h = 5381;
            IceInternal.HashUtil.hashAdd(ref h, connectionId_);
            return h;
        }

        public virtual int CompareTo(EndpointI p)
        {
            if(!connectionId_.Equals(p.connectionId_))
            {
                return string.Compare(connectionId_, p.connectionId_, StringComparison.Ordinal);
            }

            return 0;
        }

        //
        // Marshal the endpoint.
        //
        public abstract void streamWrite(BasicStream s);

        //
        // Return the endpoint type.
        //
        public abstract short type();

        //
        // Return the protocol name.
        //
        public abstract string protocol();

        //
        // Return the timeout for the endpoint in milliseconds. 0 means
        // non-blocking, -1 means no timeout.
        //
        public abstract int timeout();

        //
        // Return a new endpoint with a different timeout value, provided
        // that timeouts are supported by the endpoint. Otherwise the same
        // endpoint is returned.
        //
        public abstract EndpointI timeout(int t);

        //
        // Return a new endpoint with a different connection id.
        //
        public abstract EndpointI connectionId(string connectionId);

        //
        // Return true if the endpoints support bzip2 compress, or false
        // otherwise.
        //
        public abstract bool compress();

        //
        // Return a new endpoint with a different compression value,
        // provided that compression is supported by the
        // endpoint. Otherwise the same endpoint is returned.
        //
        public abstract EndpointI compress(bool co);

        //
        // Return true if the endpoint is datagram-based.
        //
        public abstract bool datagram();
        
        //
        // Return true if the endpoint is secure.
        //
        public abstract bool secure();

        //
        // Return the connection ID.
        //
        public string connectionId()
        {
            return connectionId_;
        }

        //
        // Return a server side transceiver for this endpoint, or null if a
        // transceiver can only be created by an acceptor. In case a
        // transceiver is created, this operation also returns a new
        // "effective" endpoint, which might differ from this endpoint,
        // for example, if a dynamic port number is assigned.
        //
        public abstract Transceiver transceiver(ref EndpointI endpoint);

        //
        // Return a connector for this endpoint, or empty list if no connector
        // is available.
        //
        public abstract List<Connector> connectors(Ice.EndpointSelectionType selType);
        public abstract void connectors_async(Ice.EndpointSelectionType selType, EndpointI_connectors callback);

        //
        // Return an acceptor for this endpoint, or null if no acceptors
        // is available. In case an acceptor is created, this operation
        // also returns a new "effective" endpoint, which might differ
        // from this endpoint, for example, if a dynamic port number is
        // assigned.
        //
        public abstract Acceptor acceptor(ref EndpointI endpoint, string adapterName);

        //
        // Expand endpoint out in to separate endpoints for each local
        // host if listening on INADDR_ANY on server side or if no host
        // was specified on client side.
        //
        public abstract List<EndpointI> expand();
 
        //
        // Check whether the endpoint is equivalent to another one.
        //
        public abstract bool equivalent(EndpointI endpoint);

        public virtual List<Connector> connectors(List<EndPoint> addresses, NetworkProxy proxy)
        {
            Debug.Assert(false);
            return null;
        }

        protected void
        parseOption(string option, string arg, string desc, string str)
        {
            if(option.Equals("-v"))
            {
                if(arg == null)
                {
                    throw new Ice.EndpointParseException("no argument provided for -v option in endpoint `" +
                                                         desc + " "+ str + "'");
                }

                try
                {
                    protocol_ = Ice.Util.stringToProtocolVersion(arg);
                }
                catch(Ice.VersionParseException e)
                {
                    throw new Ice.EndpointParseException("invalid protocol version `" + arg + "' in endpoint `" +
                                                         desc + " "+ str + "':\n" + e.str);
                }
            }            
            else if(option.Equals("-e"))
            {
                if(arg == null)
                {
                    throw new Ice.EndpointParseException("no argument provided for -e option in endpoint `" +
                                                         desc + " " + str + "'");
                }
            
                try
                {
                    encoding_ = Ice.Util.stringToEncodingVersion(arg);
                }
                catch(Ice.VersionParseException e)
                {
                    throw new Ice.EndpointParseException("invalid encoding version `" + arg + "' in endpoint `" +
                                                         desc + " "+ str + "':\n" + e.str);
                }
            }
            else
            {
                throw new Ice.EndpointParseException("unknown option `" + option + "' in `" + desc + " " + str + "'");
            }
        }

        protected Ice.ProtocolVersion protocol_;
        protected Ice.EncodingVersion encoding_;
        protected string connectionId_ = "";
    }

}
