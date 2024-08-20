//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_INCOMING_REQUEST_H
#define ICE_INCOMING_REQUEST_H

#include "Config.h"
#include "ConnectionF.h"
#include "Current.h"
#include "ObjectAdapterF.h"

namespace Ice
{
    class InputStream;

    /**
     * Represent a request received by a connection. It's the argument to the dispatch function on Object.
     * @remarks IncomingRequest is neither copyable nor movable. It can be used only on the dispatch thread.
     * @see Object::dispatch
     * \headerfile Ice/Ice.h
     */
    class ICE_API IncomingRequest final
    {
    public:
        /**
         * Construct an IncomingRequest object.
         * @param requestId The request ID. It's 0 for oneway requests.
         * @param connection The connection that received the request. It's null for collocated invocations.
         * @param adapter The object adapter to set in Current.
         * @param inputStream The input stream buffer over the incoming Ice protocol request message. The stream is
         * positioned at the beginning of the request header - the next data to read is the identity of the target.
         * @param batchRequestCount When the incoming request is a batch request with two or more requests, provides the
         * total number of requests in the batch request.
         * @remarks This constructor reads the request header from inputStream. When it completes, the input stream is
         * positioned at the beginning of encapsulation carried by the request.
         */
        IncomingRequest(
            int32_t requestId,
            ConnectionPtr connection,
            ObjectAdapterPtr adapter,
            InputStream& inputStream,
            std::optional<std::int32_t> batchRequestCount);

        IncomingRequest(const IncomingRequest&) = delete;
        IncomingRequest(IncomingRequest&&) noexcept = delete;
        IncomingRequest& operator=(const IncomingRequest&) = delete;
        IncomingRequest& operator=(IncomingRequest&&) = delete;

        /**
         * Get the current object of the request.
         * @return A reference to the current object of the request.
         */
        Current& current() noexcept { return _current; }

        /**
         * Get the current object of the request.
         * @return A const reference to the current object of the request.
         */
        const Ice::Current& current() const noexcept { return _current; }

        /**
         * Get the input stream buffer of the request.
         * @return A reference to the input stream buffer.
         */
        InputStream& inputStream() noexcept { return _inputStream; }

        /**
         * Get the number of bytes in the request.
         * @return The number of bytes in the request. These are all the bytes starting with the identity of the target.
         */
        std::int32_t size() const { return _requestSize; }

        /**
         * Get the batch request count.
         * @return The batch request count, or nullopt if the request was not received in a batch or was received in a
         * 1-request batch.
         */
        std::optional<std::int32_t> batchRequestCount() const noexcept { return _batchRequestCount; }

    private:
        InputStream& _inputStream;
        Current _current;
        std::int32_t _requestSize;
        std::optional<std::int32_t> _batchRequestCount;
    };
}

#endif
