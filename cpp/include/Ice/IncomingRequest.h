// Copyright (c) ZeroC, Inc.

#ifndef ICE_INCOMING_REQUEST_H
#define ICE_INCOMING_REQUEST_H

#include "Config.h"
#include "ConnectionF.h"
#include "Current.h"
#include "ObjectAdapterF.h"

namespace Ice
{
    class InputStream;

    /// Represents a request received by a connection. It's the argument to the dispatch function on Object.
    /// @remark IncomingRequest is neither copyable nor movable. It can be used only on the dispatch thread.
    /// @see Object::dispatch
    /// @headerfile Ice/Ice.h
    class ICE_API IncomingRequest final
    {
    public:
        /// Construct an IncomingRequest object.
        /// @param requestId The request ID. It's 0 for oneway requests.
        /// @param connection The connection that received the request. It's null for collocated invocations.
        /// @param adapter The object adapter to set in Current.
        /// @param inputStream The input stream buffer over the incoming Ice protocol request message. The stream is
        /// positioned at the beginning of the request header - the next data to read is the identity of the target.
        /// @param requestCount The number of requests remaining in the InputStream. This value is always 1 for
        /// non-batched requests. For batch requests, it is the number of requests remaining in the InputStream.
        /// @remark This constructor reads the request header from inputStream. When it completes, the input stream is
        /// positioned at the beginning of encapsulation carried by the request.
        IncomingRequest(
            int32_t requestId,
            ConnectionPtr connection,
            ObjectAdapterPtr adapter,
            InputStream& inputStream,
            std::int32_t requestCount);

        IncomingRequest(const IncomingRequest&) = delete;
        IncomingRequest(IncomingRequest&&) noexcept = delete;
        IncomingRequest& operator=(const IncomingRequest&) = delete;
        IncomingRequest& operator=(IncomingRequest&&) = delete;

        /// Get the current object of the request.
        /// @return A reference to the current object of the request.
        Current& current() noexcept { return _current; }

        /// Get the current object of the request.
        /// @return A const reference to the current object of the request.
        [[nodiscard]] const Ice::Current& current() const noexcept { return _current; }

        /// Get the input stream buffer of the request.
        /// @return A reference to the input stream buffer.
        InputStream& inputStream() noexcept { return _inputStream; }

        /// Get the number of bytes in the request.
        /// @return The number of bytes in the request. These are all the bytes starting with the identity of the
        /// target.
        [[nodiscard]] std::int32_t size() const { return _requestSize; }

        /// Get the request count.
        /// @return The value is always 1 for non-batched requests. For batch requests, it is the number
        /// of requests remaining in the InputStream.
        [[nodiscard]] std::int32_t requestCount() const noexcept { return _requestCount; }

    private:
        InputStream& _inputStream;
        Current _current;
        std::int32_t _requestSize;
        std::int32_t _requestCount;
    };
}

#endif
