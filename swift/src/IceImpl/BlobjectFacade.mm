//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "BlobjectFacade.h"
#import "ObjectAdapter.h"
#import "Convert.h"

#import "Connection.h"

void
BlobjectFacade::ice_invokeAsync(
    std::pair<const std::uint8_t*, const std::uint8_t*> inEncaps,
    std::function<void(bool, const std::pair<const std::uint8_t*, const std::uint8_t*>&)> response,
    std::function<void(std::exception_ptr)> error,
    const Ice::Current& current)
{
    ICEBlobjectResponse responseCallback = ^(bool ok, const void* outParams, long count) {
      const std::uint8_t* start = static_cast<const std::uint8_t*>(outParams);
      response(ok, std::make_pair(start, start + static_cast<size_t>(count)));
    };

    ICEBlobjectException exceptionCallback = ^(ICERuntimeException* e) {
      error(convertException(e));
    };

    ICEObjectAdapter* adapter = [ICEObjectAdapter getHandle:current.adapter];
    ICEConnection* con = [ICEConnection getHandle:current.con];

    @autoreleasepool
    {
        [_facade facadeInvoke:adapter
                inEncapsBytes:const_cast<std::uint8_t*>(inEncaps.first)
                inEncapsCount:static_cast<long>(inEncaps.second - inEncaps.first)
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
