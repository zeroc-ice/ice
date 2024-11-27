//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_OUTGOING_RESPONSE_H
#define ICE_OUTGOING_RESPONSE_H

#include "Current.h"
#include "MarshaledResult.h"
#include "OutputStream.h"

#include <functional>

namespace Ice
{
    /**
     * Represents the status of a response.
     * \headerfile Ice/Ice.h
     */
    enum class ReplyStatus : std::uint8_t
    {
        Ok = 0,
        UserException = 1,
        ObjectNotExist = 2,
        FacetNotExist = 3,
        OperationNotExist = 4,
        UnknownLocalException = 5,
        UnknownUserException = 6,
        UnknownException = 7
    };

    /**
     * Represents the response to an incoming request. It's the argument to the sendResponse callback accepted by
     * Object::dispatch.
     * @remarks OutgoingResponse is movable but not copyable. sendResponse wrappers must move the response to the next
     * callback.
     * @see Object::dispatch
     * \headerfile Ice/Ice.h
     */
    class ICE_API OutgoingResponse final
    {
    public:
        /**
         * Construct an OutgoingResponse object.
         * @param replyStatus The status of the response.
         * @param exceptionId The ID of the exception, when the response carries an exception.
         * @param exceptionDetails The full details of the exception, when the response carries an exception.
         * @param outputStream The output stream that holds the response.
         * @param current A reference to the current object of the request.
         */
        OutgoingResponse(
            ReplyStatus replyStatus,
            std::string exceptionId,
            std::string exceptionDetails,
            OutputStream outputStream,
            const Current& current) noexcept;

        /**
         * Construct an OutgoingResponse object with the Ok reply status.
         * @param outputStream The output stream that holds the response.
         * @param current A reference to the current object of the request.
         */
        OutgoingResponse(OutputStream outputStream, const Current& current) noexcept
            : OutgoingResponse(ReplyStatus::Ok, "", "", std::move(outputStream), current)
        {
        }

        /**
         * Move constructor.
         * @param other The response to move into this new response.
         */
        OutgoingResponse(OutgoingResponse&& other) noexcept = default;

        /**
         * Move assignment operator.
         * @param other The response to move into this response.
         */
        OutgoingResponse& operator=(OutgoingResponse&& other) noexcept = default;

        OutgoingResponse(const OutgoingResponse&) = delete;
        OutgoingResponse& operator=(const OutgoingResponse&) = delete;

        /**
         * Get the current object of this response.
         * @return A const reference to the current object.
         * @remarks The response only holds onto a reference for this Current object. The caller keeps the Current
         * object alive until the call to sendResponse completes.
         */
        const Current& current() const noexcept { return _current.get(); }

        /**
         * Get the exception ID of the response.
         * @return The exception ID of the response. It's empty when replyStatus() is ReplyStatus::Ok. Otherwise, this
         * ID is the Slice type ID of the exception marshaled into this response if this exception was defined in Slice
         * or is derived from Ice::LocalException. For other exceptions, this ID is the value returned by
         * std::exception::what().
         */
        const std::string& exceptionId() const noexcept { return _exceptionId; }

        /**
         * Get the full details of the exception marshaled into the response.
         * @return The exception details. For Ice exceptions, it's usually produced by streaming the exception into
         * a std::ostringstream with operator<<. It's empty when replyStatus() is ReplyStatus::Ok.
         */
        const std::string& exceptionDetails() const noexcept { return _exceptionDetails; }

        /**
         * Get the reply status of the response.
         * @return The reply status.
         */
        ReplyStatus replyStatus() const noexcept { return _replyStatus; }

        /**
         * Get the output stream buffer of the response.
         * @return A reference to the output stream buffer.
         */
        OutputStream& outputStream() noexcept { return _outputStream; }

        /**
         * Get the number of bytes in the response.
         * @return The number of bytes in the response.
         */
        std::int32_t size() const noexcept;

    private:
        std::reference_wrapper<const Current> _current;
        std::string _exceptionId;
        std::string _exceptionDetails;
        OutputStream _outputStream;
        ReplyStatus _replyStatus;
    };

    /**
     * Create an OutgoingResponse object with the Ok reply status.
     * @param marshal A function that writes the payload of the response to the output stream.
     * @param current The current object of the incoming request.
     * @param format The class format to use when marshaling the response.
     * @return The new response.
     */
    ICE_API OutgoingResponse makeOutgoingResponse(
        std::function<void(OutputStream*)> marshal,
        const Current& current,
        std::optional<FormatType> format = std::nullopt) noexcept;

    /**
     * Create an OutgoingResponse object with the Ok reply status and an empty payload.
     * @param current A reference to the current object of the request.
     * @return The new response.
     */
    ICE_API OutgoingResponse makeEmptyOutgoingResponse(const Current& current) noexcept;

    /**
     * Create an OutgoingResponse object with the Ok or UserException reply status.
     * @param ok When true, the reply status is Ok. When false, the reply status is UserException.
     * @param encapsulation The payload-encapsulation of the response or the user exception. It should be encoded using
     * Current::encoding but this function does not verify it.
     * @param current A reference to the current object of the request.
     * @return The new response.
     */
    ICE_API OutgoingResponse makeOutgoingResponse(
        bool ok,
        std::pair<const std::byte*, const std::byte*> encapsulation,
        const Current& current) noexcept;

    /**
     * Create an OutgoingResponse object with a reply status other than Ok.
     * @param exception The exception to marshal into the response.
     * @param current A reference to the current object of the request.
     * @return The new response.
     */
    ICE_API OutgoingResponse makeOutgoingResponse(std::exception_ptr exception, const Current& current) noexcept;
}

#endif
