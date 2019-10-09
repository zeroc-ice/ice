//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "BlobjectFacade.h"
#import "ObjectAdapter.h"
#import "Convert.h"

#import "Connection.h"

void
BlobjectFacade::ice_invokeAsync(std::pair<const Byte*, const Byte*> inEncaps,
                                std::function<void(bool, const std::pair<const Byte*, const Byte*>&)> response,
                                std::function<void(std::exception_ptr)> error,
                                const Ice::Current& current)
{
    ICEBlobjectResponse responseCallback = ^(bool ok, const void* outParams, long count) {
        const Ice::Byte* start = static_cast<const Ice::Byte*>(outParams);
        response(ok, std::make_pair(start, start + static_cast<size_t>(count)));
    };

    ICEBlobjectException exceptionCallback = ^(ICERuntimeException* e) {
        error(convertException(e));
    };

    ICEObjectAdapter* adapter = [ICEObjectAdapter getHandle:current.adapter];
    ICEConnection* con = [ICEConnection getHandle:current.con];

    [_facade facadeInvoke:adapter
            inEncapsBytes:const_cast<Ice::Byte*>(inEncaps.first)
            inEncapsCount:static_cast<long>(inEncaps.second - inEncaps.first)
                      con:con
                     name:toNSString(current.id.name) category:toNSString(current.id.category)
                    facet:toNSString(current.facet)
                operation:toNSString(current.operation)
                     mode:static_cast<uint8_t>(current.mode)
                  context:toNSDictionary(current.ctx)
                requestId:current.requestId
            encodingMajor:current.encoding.major
            encodingMinor:current.encoding.minor
                 response:responseCallback
                exception:exceptionCallback];
}
