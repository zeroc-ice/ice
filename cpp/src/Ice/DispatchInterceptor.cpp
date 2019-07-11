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
    catch(const std::exception&)
    {
        //
        // If the input parameters weren't read, make sure we skip them here. It's needed to read the
        // encoding version used by the client to eventually marshal the user exception. It's also needed
        // if we are dispatch a batch oneway request to read the next batch request.
        //
        if(in.getCurrent().encoding.major == 0 && in.getCurrent().encoding.minor == 0)
        {
            in.skipReadParams();
        }
        throw;
    }
}
