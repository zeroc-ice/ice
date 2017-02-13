// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

bool
Ice::DispatchInterceptor::_iceDispatch(IceInternal::Incoming& in, const Current& /*current*/)
{
    try
    {
        IncomingRequest request(in);
        return dispatch(request);
    }
    catch(const ResponseSentException&)
    {
        return false;
    }
}
