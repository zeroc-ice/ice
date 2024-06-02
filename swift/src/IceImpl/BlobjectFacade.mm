//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "Ice/AsyncResponseHandler.h"
#import "BlobjectFacade.h"
#import "Convert.h"
#import "Exception.h"
#import "ObjectAdapter.h"

#import "Connection.h"

void
SwiftDispatcher::dispatch(Ice::IncomingRequest& request, std::function<void(Ice::OutgoingResponse)> sendResponse)
{
    Ice::Current& current = request.current();

    // Here, we assume we can't guarantee that the callbacks are only called once. Hopefully we can fix that.
    auto responseHandler = make_shared<IceInternal::AsyncResponseHandler>(std::move(sendResponse), current);

    ICEBlobjectResponse responseCallback = ^(bool ok, const void* outParams, long count) {
      const std::byte* start = static_cast<const std::byte*>(outParams);
      responseHandler->sendResponse(ok, std::make_pair(start, start + static_cast<size_t>(count)));
    };

    ICEBlobjectException exceptionCallback = ^(ICEDispatchException* e) {
      responseHandler->sendException(e.cppExceptionPtr);
    };

    ICEObjectAdapter* adapter = [ICEObjectAdapter getHandle:current.adapter];
    ICEConnection* con = [ICEConnection getHandle:current.con];

    int32_t sz;
    const std::byte* inEncaps;
    request.inputStream().readEncapsulation(inEncaps, sz);

    @autoreleasepool
    {
        [_facade facadeInvoke:adapter
                inEncapsBytes:const_cast<std::byte*>(inEncaps)
                inEncapsCount:static_cast<long>(sz)
                          con:con
                         name:toNSString(current.id.name)
                     category:toNSString(current.id.category)
                        facet:toNSString(current.facet)
                    operation:toNSString(current.operation)
                         mode:static_cast<std::uint8_t>(current.mode)
                      context:toNSDictionary(current.ctx)
                    requestId:current.requestId
                encodingMajor:current.encoding.major
                encodingMinor:current.encoding.minor
                     response:responseCallback
                    exception:exceptionCallback];
    }
}
