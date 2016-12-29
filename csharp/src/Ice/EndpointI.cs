// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Collections.Generic;
    using System.Diagnostics;
    using System;

    public interface EndpointI_connectors
    {
        void connectors(List<Connector> connectors);
        void exception(Ice.LocalException ex);
    }

    public abstract class EndpointI : Ice.Endpoint, IComparable<EndpointI>
    {
        public override string ToString()
        {
            return ice_toString_();
        }

        public virtual string ice_toString_()
        {
            //
            // WARNING: Certain features, such as proxy validation in Glacier2,
            // depend on the format of proxy strings. Changes to toString() and
            // methods called to generate parts of the reference string could break
            // these features. Please review for all features that depend on the
            // format of proxyToString() before changing this and related code.
            //
            return protocol() + options();
        }

        public abstract Ice.EndpointInfo getInfo();

        public override bool Equals(object obj)
        {
            if(!(obj is EndpointI))
            {
                return false;
            }
            return CompareTo((EndpointI)obj) == 0;
        }

        public override int GetHashCode() // Avoids a compiler warning.
        {
            Debug.Assert(false);
            return 0;
        }

        //
        // Marshal the endpoint.
        //
        virtual public void streamWrite(Ice.OutputStream s)
        {
            s.startEncapsulation();
            streamWriteImpl(s);
            s.endEncapsulation();
        }
        public abstract void streamWriteImpl(Ice.OutputStream s);

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
        // Return the connection ID.
        //
        public abstract string connectionId();

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
        // Return a server side transceiver for this endpoint, or null if a
        // transceiver can only be created by an acceptor.
        //
        public abstract Transceiver transceiver();

        //
        // Return a connector for this endpoint, or empty list if no connector
        // is available.
        //
        public abstract void connectors_async(Ice.EndpointSelectionType selType, EndpointI_connectors callback);

        //
        // Return an acceptor for this endpoint, or null if no acceptors
        // is available.
        //
        public abstract Acceptor acceptor(string adapterName);

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

        public abstract int CompareTo(EndpointI obj);

        public abstract string options();

        public virtual void initWithOptions(List<string> args)
        {
            List<string> unknown = new List<string>();

            string str = "`" + protocol() + " ";
            foreach(string p in args)
            {
                if(IceUtilInternal.StringUtil.findFirstOf(p, " \t\n\r") != -1)
                {
                    str += " \"" + p + "\"";
                }
                else
                {
                    str += " " + p;
                }
            }
            str += "'";

            for(int n = 0; n < args.Count; ++n)
            {
                string option = args[n];
                if(option.Length < 2 || option[0] != '-')
                {
                    unknown.Add(option);
                    continue;
                }

                string argument = null;
                if(n + 1 < args.Count && args[n + 1][0] != '-')
                {
                    argument = args[++n];
                }

                if(!checkOption(option, argument, str))
                {
                    unknown.Add(option);
                    if(argument != null)
                    {
                        unknown.Add(argument);
                    }
                }
            }

            args.Clear();
            args.AddRange(unknown);
        }

        protected virtual bool checkOption(string option, string argument, string endpoint)
        {
            // Must be overridden to check for options.
            return false;
        }
    }

}
