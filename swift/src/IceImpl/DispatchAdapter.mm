// Copyright (c) ZeroC, Inc.
#import "DispatchAdapter.h"
#import "Connection.h"
#import "Convert.h"
#import "Exception.h"
#import "Ice/AsyncResponseHandler.h"
#import "ObjectAdapter.h"

void
CppDispatcher::dispatch(Ice::IncomingRequest& request, std::function<void(Ice::OutgoingResponse)> sendResponse)
{
    // Captured as a const copy by the block according to https://clang.llvm.org/docs/BlockLanguageSpec.html
    Ice::Current current{request.current()};

    ICEOutgoingResponse outgoingResponse =
        ^(uint8_t replyStatus, NSString* exceptionId, NSString* exceptionMessage, const void* message, long count) {
          // We need to copy the message here as we don't own the memory and it can be sent asynchronously.
          Ice::OutputStream ostr(current.adapter->getCommunicator());
          ostr.writeBlob(static_cast<const std::byte*>(message), static_cast<size_t>(count));

          sendResponse(Ice::OutgoingResponse{
              static_cast<Ice::ReplyStatus>(replyStatus),
              fromNSString(exceptionId),
              fromNSString(exceptionMessage),
              std::move(ostr),
              current});
        };

    int32_t sz;
    const std::byte* inEncaps;
    request.inputStream().readEncapsulation(inEncaps, sz);

    ICEObjectAdapter* adapter = [ICEObjectAdapter getHandle:current.adapter];
    ICEConnection* con = [ICEConnection getHandle:current.con];

    @autoreleasepool
    {
        [_dispatchAdapter dispatch:adapter
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
                 completionHandler:outgoingResponse];
    }
}
