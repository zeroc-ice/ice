// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import "IceObjcBlobjectFacade.h"
#import "IceObjcObjectAdapter.h"
#import "IceObjcUtil.h"

#import "IceObjcInputStream.h"
#import "IceObjcConnection.h"

void
BlobjectFacade::ice_invokeAsync(std::pair<const Byte*, const Byte*> inEncaps,
                                         std::function<void(bool, const std::pair<const Byte*, const Byte*>&)> response,
                                         std::function<void(std::exception_ptr)> error,
                                         const Ice::Current& current)
{
    void(^responseCallback)(bool, const void*, size_t) = ^(bool ok, const void* outParams, size_t outSize) {
        const Ice::Byte* start = reinterpret_cast<const Ice::Byte*>(outParams);
        response(ok, std::make_pair(start, start + outSize));
    };

    void(^exceptionCallback)(ICERuntimeException*) = ^(ICERuntimeException* e) {
        error(convertException(e));
    };

    // Copy the bytes
    std::vector<Ice::Byte> inBytes(inEncaps.first, inEncaps.second);

    ICEObjectAdapter* adapter = createLocalObject(current.adapter, [&current] () -> id
                                                  {
                                                      return [[ICEObjectAdapter alloc] initWithCppObjectAdapter:current.adapter];
                                                  });
    [_facade facadeInvoke:adapter
                       is:[[ICEInputStream alloc] initWithBytes:std::move(inBytes)]
                      con: current.con ? [[ICEConnection alloc] initWithCppConnection:current.con] : nil
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
