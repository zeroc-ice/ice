//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace IceInternal
{
    public interface IEndpointConnectors
    {
        void Connectors(List<IConnector> connectors);
        void Exception(Ice.LocalException ex);
    }

    public abstract class Endpoint : Ice.IEndpoint, IComparable<Endpoint>, IEquatable<Endpoint>
    {
        public override string ToString()
        {
            //
            // WARNING: Certain features, such as proxy validation in Glacier2,
            // depend on the format of proxy strings. Changes to toString() and
            // methods called to generate parts of the reference string could break
            // these features. Please review for all features that depend on the
            // format of proxyToString() before changing this and related code.
            //
            return Protocol() + Options();
        }

        public abstract Ice.EndpointInfo GetInfo();

        public override bool Equals(object obj) => obj != null && obj is Endpoint other && Equals(other);

        public bool Equals(Endpoint other) => CompareTo(other) == 0;

        public abstract override int GetHashCode(); // Avoids a compiler warning.
        //
        // Marshal the endpoint.
        //
        public virtual void StreamWrite(Ice.OutputStream s)
        {
            s.StartEndpointEncapsulation();
            StreamWriteImpl(s);
            s.EndEndpointEncapsulation();
        }
        public abstract void StreamWriteImpl(Ice.OutputStream s);

        //
        // Return the endpoint type.
        //
        public abstract short Type();

        //
        // Return the protocol name.
        //
        public abstract string Protocol();

        //
        // Return the timeout for the endpoint in milliseconds. 0 means
        // non-blocking, -1 means no timeout.
        //
        public abstract int Timeout();

        //
        // Return a new endpoint with a different timeout value, provided
        // that timeouts are supported by the endpoint. Otherwise the same
        // endpoint is returned.
        //
        public abstract Endpoint Timeout(int t);

        //
        // Return the connection ID.
        //
        public abstract string ConnectionId();

        //
        // Return a new endpoint with a different connection id.
        //
        public abstract Endpoint ConnectionId(string connectionId);

        //
        // Return true if the endpoints support bzip2 compress, or false
        // otherwise.
        //
        public abstract bool Compress();

        //
        // Return a new endpoint with a different compression value,
        // provided that compression is supported by the
        // endpoint. Otherwise the same endpoint is returned.
        //
        public abstract Endpoint Compress(bool co);

        //
        // Return true if the endpoint is datagram-based.
        //
        public abstract bool Datagram();

        //
        // Return true if the endpoint is secure.
        //
        public abstract bool Secure();

        //
        // Return a server side transceiver for this endpoint, or null if a
        // transceiver can only be created by an acceptor.
        //
        public abstract ITransceiver? Transceiver();

        //
        // Return a connector for this endpoint, or empty list if no connector
        // is available.
        //
        public abstract void ConnectorsAsync(Ice.EndpointSelectionType selType, IEndpointConnectors callback);

        //
        // Return an acceptor for this endpoint, or null if no acceptors
        // is available.
        //
        public abstract IAcceptor? Acceptor(string adapterName);

        //
        // Expand endpoint out in to separate endpoints for each local
        // host if listening on INADDR_ANY on server side or if no host
        // was specified on client side.
        //
        public abstract List<Endpoint> ExpandIfWildcard();

        //
        // Expand endpoint out into separate endpoints for each IP
        // address returned by the DNS resolver. Also returns the
        // endpoint which can be used to connect to the returned
        // endpoints or null if no specific endpoint can be used to
        // connect to these endpoints (e.g.: with the IP endpoint,
        // it returns this endpoint if it uses a fixed port, null
        // otherwise).
        //
        public abstract List<Endpoint> ExpandHost(out Endpoint? publishedEndpoint);

        //
        // Check whether the endpoint is equivalent to another one.
        //
        public abstract bool Equivalent(Endpoint endpoint);

        public abstract int CompareTo(Endpoint obj);

        public abstract string Options();

        public virtual void InitWithOptions(List<string> args)
        {
            var unknown = new List<string>();

            string str = "`" + Protocol() + " ";
            foreach (string p in args)
            {
                if (IceUtilInternal.StringUtil.FindFirstOf(p, " \t\n\r") != -1)
                {
                    str += " \"" + p + "\"";
                }
                else
                {
                    str += " " + p;
                }
            }
            str += "'";

            for (int n = 0; n < args.Count; ++n)
            {
                string option = args[n];
                if (option.Length < 2 || option[0] != '-')
                {
                    unknown.Add(option);
                    continue;
                }

                string? argument = null;
                if (n + 1 < args.Count && args[n + 1][0] != '-')
                {
                    argument = args[++n];
                }

                if (!CheckOption(option, argument, str))
                {
                    unknown.Add(option);
                    if (argument != null)
                    {
                        unknown.Add(argument);
                    }
                }
            }

            args.Clear();
            args.AddRange(unknown);
        }

        protected virtual bool CheckOption(string option, string? argument, string endpoint) =>
            // Must be overridden to check for options.
            false;
    }

}
