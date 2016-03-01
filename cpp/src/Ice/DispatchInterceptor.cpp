// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/DispatchInterceptor.h>
#include <Ice/IncomingRequest.h>
#include <Ice/LocalException.h>

using namespace Ice;
using namespace IceInternal;

DispatchStatus
Ice::DispatchInterceptor::__dispatch(IceInternal::Incoming& in, const Current& /*current*/)
{
    try
    {
        IncomingRequest request(in);
        DispatchStatus status = dispatch(request);
        if(status != DispatchAsync)
        {
            //
            // Make sure 'in' owns the connection etc.
            //
            in.killAsync();
        }
        return status;
    }
    catch(const ResponseSentException&)
    {
        return DispatchAsync;
    }
    catch(...)
    {
        try
        {
            in.killAsync();
        }
        catch(const ResponseSentException&)
        {
            return DispatchAsync;
        }
        throw;
    }
}
