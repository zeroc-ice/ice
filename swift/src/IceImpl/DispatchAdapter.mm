// Copyright (c) ZeroC, Inc.
#import "include/DispatchAdapter.h"
#import "Convert.h"
#import "Ice/AsyncResponseHandler.h"
#import "include/Connection.h"
#import "include/LocalExceptionFactory.h"
#import "include/ObjectAdapter.h"

void
CppDispatcher::dispatch(Ice::IncomingRequest& request, std::function<void(Ice::OutgoingResponse)> sendResponse)
{
    // Captured as a const copy by the block according to https://clang.llvm.org/docs/BlockLanguageSpec.html
    Ice::Current current{request.current()};

    int32_t sz;
    const std::byte* inEncaps;
    std::function<void()> cleanup;

    // An InputSteam can contain one or more requests when we're processing a batch request. In the case when there
    // is more than one request in the the batch we need to copy the encapsulation from the InputStream to a
    // heap allocated vector as the remainder of the memory is needed for subsequent requests.
    if (request.requestCount() > 1)
    {
        Ice::InputStream& inputStream = request.inputStream();
        inputStream.readEncapsulation(inEncaps, sz);

        // Copy the contents to a heap allocated vector.
        auto encapsulation = new std::vector<std::byte>(inEncaps, inEncaps + sz);
        inEncaps = encapsulation->data();

        cleanup = [encapsulation] { delete encapsulation; };
    }
    else
    {
        // In the case of a twoway request we can just take the memory as its no longer needed after this request.
        // Move the request's InputStream into a new stack allocated InputStream.
        // When dispatch completes, the new InputStream will be deleted.
        auto dispatchInputStream = new Ice::InputStream(std::move(request.inputStream()));

        cleanup = [dispatchInputStream] { delete dispatchInputStream; };

        dispatchInputStream->readEncapsulation(inEncaps, sz);
    };

    ICEOutgoingResponse outgoingResponse =
        ^(uint8_t replyStatus, NSString* exceptionId, NSString* exceptionDetails, const void* message, long count) {
          cleanup();

          // We need to copy the message here as we don't own the memory and it can be sent asynchronously.
          Ice::OutputStream ostr(current.adapter->getCommunicator());
          ostr.writeBlob(static_cast<const std::byte*>(message), static_cast<size_t>(count));

          sendResponse(Ice::OutgoingResponse{
              static_cast<Ice::ReplyStatus>(replyStatus),
              fromNSString(exceptionId),
              fromNSString(exceptionDetails),
              std::move(ostr),
              current});
        };

    ICEObjectAdapter* adapter = [ICEObjectAdapter getHandle:current.adapter];
    ICEConnection* con = [ICEConnection getHandle:current.con];

    // Both are null (colloc) or both are non-null (non-colloc).
    assert((current.con && con) || (!current.con && !con));

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
            outgoingResponseHandler:outgoingResponse];
    }
}
