// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Collections;

    public abstract class EndpointI : Ice.Endpoint, System.IComparable
    {
        public int ice_hash() // From LocalObject.
        {
            return GetHashCode();
        }

        public object Clone() // From LocalObject.
        {
            return MemberwiseClone();
        }

	public override string ToString()
        {
	    return ice_toString_();
	}

	public abstract string ice_toString_();
	public abstract int CompareTo(object obj);

	//
	// Marshal the endpoint.
	//
	public abstract void streamWrite(BasicStream s);

	//
	// Return the endpoint type.
	//
	public abstract short type();

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
	// Return true if the endpoint type is unknown.
	//
	public abstract bool unknown();

	//
	// Return a client side transceiver for this endpoint, or null if a
	// transceiver can only be created by a connector.
	//
	public abstract Transceiver clientTransceiver();

	//
	// Return a server side transceiver for this endpoint, or null if a
	// transceiver can only be created by an acceptor. In case a
	// transceiver is created, this operation also returns a new
	// "effective" endpoint, which might differ from this endpoint,
	// for example, if a dynamic port number is assigned.
	//
	public abstract Transceiver serverTransceiver(ref EndpointI endpoint);

	//
	// Return a connector for this endpoint, or null if no connector
	// is available.
	//
	public abstract Connector connector();

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
        // host if endpoint was configured with no host set.
        //
        public abstract ArrayList expand(bool includeLoopback);
 
        //
        // Return whether the endpoint should be published in proxies
        // created by the Object Adapter.
        //
	public abstract bool publish();

	//
	// Check whether the endpoint is equivalent to a specific
	// Transceiver or Acceptor.
	//
	public abstract bool equivalent(Transceiver transceiver);
	public abstract bool equivalent(Acceptor acceptor);
    }

}
