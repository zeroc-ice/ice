// Copyright (c) ZeroC, Inc.

#ifndef ICE_OUTGOING_RESPONSE_H
#define ICE_OUTGOING_RESPONSE_H

#include "Current.h"
#include "Ice/ReplyStatus.h"
#include "MarshaledResult.h"
#include "OutputStream.h"

#include <functional>

namespace Ice
{
    /// Represents the response to an incoming request. It's the argument to the `sendResponse` callback accepted by
    /// Object::dispatch.
    /// @remark OutgoingResponse is movable but not copyable. `sendResponse` wrappers must move the response to the next
    /// callback.
    /// @see Object::dispatch
    /// @headerfile Ice/Ice.h
    class ICE_API OutgoingResponse final
    {
    public:
        /// Constructs an OutgoingResponse object.
        /// @param replyStatus The status of the response.
        /// @param exceptionId The type ID of the exception, when the response carries an exception other than a user
        /// exception.
        /// @param exceptionDetails The full details of the exception, when the response carries an exception other than
        /// a user exception.
        /// @param outputStream The output stream that holds the response.
        /// @param current A reference to the Current object of the request.
        OutgoingResponse(
            ReplyStatus replyStatus,
            std::string exceptionId,
            std::string exceptionDetails,
            OutputStream outputStream,
            const Current& current) noexcept;

        /// Constructs an OutgoingResponse object with ReplyStatus::Ok.
        /// @param outputStream The output stream that holds the response.
        /// @param current A reference to the Current object of the request.
        OutgoingResponse(OutputStream outputStream, const Current& current) noexcept
            : OutgoingResponse(ReplyStatus::Ok, "", "", std::move(outputStream), current)
        {
        }

        /// Move constructor.
        /// @param other The response to move into this new response.
        OutgoingResponse(OutgoingResponse&& other) noexcept = default;

        /// Move assignment operator.
        /// @param other The response to move into this response.
        OutgoingResponse& operator=(OutgoingResponse&& other) noexcept = default;

        OutgoingResponse(const OutgoingResponse&) = delete;
        OutgoingResponse& operator=(const OutgoingResponse&) = delete;

        /// Gets the Current object of this response.
        /// @return A const reference to the Current object.
        /// @remark The response only holds onto a reference for this Current object. The caller keeps the Current
        /// object alive until the call to `sendResponse` completes.
        [[nodiscard]] const Current& current() const noexcept { return _current.get(); }

        /// Gets the exception ID of the response.
        /// @return The exception ID of the response. It's empty when #replyStatus is ReplyStatus::Ok or
        /// ReplyStatus::UserException. Otherwise, this ID is the value returned by LocalException#ice_id. For other
        /// exceptions, this ID is the value returned by `std::exception::what()`.
        [[nodiscard]] const std::string& exceptionId() const noexcept { return _exceptionId; }

        /// Gets the full details of the exception marshaled into the response.
        /// @return The exception details. For Ice exceptions, it's produced by Exception::ice_print. It's empty
        /// when #replyStatus is ReplyStatus::Ok or ReplyStatus::UserException.
        [[nodiscard]] const std::string& exceptionDetails() const noexcept { return _exceptionDetails; }

        /// Gets the reply status of the response.
        /// @return The reply status.
        [[nodiscard]] ReplyStatus replyStatus() const noexcept { return _replyStatus; }

        /// Gets the output stream buffer of the response.
        /// @return A reference to the output stream buffer.
        OutputStream& outputStream() noexcept { return _outputStream; }

        /// Gets the number of bytes in the response.
        /// @return The number of bytes in the response.
        [[nodiscard]] std::int32_t size() const noexcept;

    private:
        std::reference_wrapper<const Current> _current;
        std::string _exceptionId;
        std::string _exceptionDetails;
        OutputStream _outputStream;
        ReplyStatus _replyStatus;
    };

    /// Creates an OutgoingResponse object with ReplyStatus::Ok.
    /// @param marshal A function that writes the payload of the response to the output stream.
    /// @param current The Current object of the incoming request.
    /// @param format The class format to use when marshaling the response.
    /// @return The new response.
    ICE_API OutgoingResponse makeOutgoingResponse(
        const std::function<void(OutputStream*)>& marshal,
        const Current& current,
        std::optional<FormatType> format = std::nullopt) noexcept;

    /// Creates an OutgoingResponse object with ReplyStatus::Ok and an empty payload.
    /// @param current A reference to the Current object of the request.
    /// @return The new response.
    ICE_API OutgoingResponse makeEmptyOutgoingResponse(const Current& current) noexcept;

    /// Creates an OutgoingResponse object with ReplyStatus::Ok or ReplyStatus::UserException.
    /// @param ok When `true`, the reply status is ReplyStatus::Ok. When false, the reply status is
    /// ReplyStatus::UserException.
    /// @param encapsulation The payload-encapsulation of the response or the user exception. It should be encoded using
    /// Current::encoding but this function does not verify it.
    /// @param current A reference to the Current object of the request.
    /// @return The new response.
    ICE_API OutgoingResponse makeOutgoingResponse(
        bool ok,
        std::pair<const std::byte*, const std::byte*> encapsulation,
        const Current& current) noexcept;

    /// Creates an OutgoingResponse object with a reply status other than ReplyStatus::Ok.
    /// @param exception The exception to marshal into the response.
    /// @param current A reference to the Current object of the request.
    /// @return The new response.
    ICE_API OutgoingResponse makeOutgoingResponse(std::exception_ptr exception, const Current& current) noexcept;
}

#endif
