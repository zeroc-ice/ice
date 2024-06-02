//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "BlobjectFacade.h"
#import "Convert.h"
#import "Exception.h"
#import "Ice/AsyncResponseHandler.h"
#import "ObjectAdapter.h"

#import "Connection.h"

void
SwiftDispatcher::dispatch(Ice::IncomingRequest& request, std::function<void(Ice::OutgoingResponse)> sendResponse)
{
    // Captured as a const copy by the block according to https://clang.llvm.org/docs/BlockLanguageSpec.html
    Ice::Current current = request.current();

    ICESendResponse sendResponseBlock =
        ^(bool ok, const void* outEncaps, size_t count, ICEDispatchException* dispatchException) {
          if (dispatchException)
          {
              sendResponse(Ice::makeOutgoingResponse(dispatchException.cppExceptionPtr, current));
          }
          else
          {
              assert(outEncaps);
              const std::byte* bytes = static_cast<const std::byte*>(outEncaps);
              sendResponse(Ice::makeOutgoingResponse(ok, std::make_pair(bytes, bytes + count), current));
          }
        };

    int32_t sz;
    const std::byte* inEncaps;
    request.inputStream().readEncapsulation(inEncaps, sz);

    ICEObjectAdapter* adapter = [ICEObjectAdapter getHandle:current.adapter];
    ICEConnection* con = [ICEConnection getHandle:current.con];

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
                 sendResponse:sendResponseBlock];
    }
}
