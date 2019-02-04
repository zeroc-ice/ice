//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
