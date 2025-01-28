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

    // The Swift side can asynchronously unmarshal the request, so we need to either move or copy the encapsulation
    // data to ensure it remains alive until Swift completes unmarshaling.
    //
    // For a batch request with multiple requests remaining in the input stream, we need to copy the encapsulation data
    // because moving it isn't an option—subsequent requests still depend on the input stream.
    //
    // For collocated requests, we also need to copy the encapsulation data because the input stream doesn't own
    // the memory. The memory is owned by the output stream used for the invocation, which might be freed before the
    // input stream is unmarshaled. Additionally, we can't take ownership of the output stream because it is still
    // needed in case of a retry.
    //
    // If neither of these conditions applies, the input stream can be safely moved to the heap for processing.
    if (request.requestCount() > 1 || request.current().con == nullptr)
    {
        Ice::InputStream& inputStream = request.inputStream();
        inputStream.readEncapsulation(inEncaps, sz);

        auto encapsulation = new std::vector<std::byte>(inEncaps, inEncaps + sz);
        inEncaps = encapsulation->data();

        cleanup = [encapsulation] { delete encapsulation; };
    }
    else
    {
        auto dispatchInputStream = new Ice::InputStream(std::move(request.inputStream()));

        cleanup = [dispatchInputStream] { delete dispatchInputStream; };

        dispatchInputStream->readEncapsulation(inEncaps, sz);
    }

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
