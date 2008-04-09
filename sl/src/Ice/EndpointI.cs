// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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
        // Return true if the endpoint type is unknown.
        //
        public abstract bool unknown();
    }
}
