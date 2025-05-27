// Copyright (c) ZeroC, Inc.

#include "Ice/OutgoingResponse.h"
#include "Ice/Demangle.h"
#include "Ice/LocalExceptions.h"
#include "Ice/ObjectAdapter.h"
#include "Ice/UserException.h"
#include "Protocol.h"

#include <typeinfo>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{
    inline string toString(const Exception& ex)
    {
        // Includes the stack trace when available (local exceptions only).
        ostringstream os;
        ex.ice_print(os);
        return os.str();
    }

    inline string createDispatchExceptionMessage(const string& typeId, const char* what)
    {
        ostringstream os;
        os << "dispatch failed with " << typeId << ": " << what;
        return os.str();
    }

    // The "core" implementation of makeOutgoingResponse for exceptions. Note that it can throw an exception.
    OutgoingResponse makeOutgoingResponseCore(std::exception_ptr exc, const Current& current)
    {
        assert(exc);
        OutputStream ostr{Ice::currentProtocolEncoding};

        if (current.requestId != 0)
        {
            ostr.writeBlob(replyHdr, sizeof(replyHdr));
            ostr.write(current.requestId);
        }
        ReplyStatus replyStatus;
        string exceptionId;
        string exceptionDetails; // may include the stack trace.
        string dispatchExceptionMessage;

        try
        {
            rethrow_exception(exc);
        }
        catch (const UserException& ex)
        {
            // We keep exceptionId and exceptionDetails empty.

            replyStatus = ReplyStatus::UserException;

            if (current.requestId != 0)
            {
                ostr.write(replyStatus);
                ostr.startEncapsulation(current.encoding, FormatType::SlicedFormat);
                ostr.write(ex);
                ostr.endEncapsulation();
            }
        }
        catch (const DispatchException& ex)
        {
            exceptionId = ex.ice_id();
            exceptionDetails = toString(ex); // can include a stack trace
            replyStatus = ex.replyStatus();

            if (replyStatus >= ReplyStatus::ObjectNotExist && replyStatus <= ReplyStatus::OperationNotExist)
            {
                if (current.requestId != 0) // only marshal response for two-way requests
                {
                    // The identity, facet, operation are often left unset at this point.
                    Identity id;
                    string facet;
                    string operation;
                    if (auto* rfe = dynamic_cast<const RequestFailedException*>(&ex))
                    {
                        id = rfe->id();
                        facet = rfe->facet();
                        operation = rfe->operation();
                    }
                    if (id.name.empty())
                    {
                        id = current.id;
                        facet = current.facet;
                    }
                    if (operation.empty())
                    {
                        operation = current.operation;
                    }

                    ostr.write(replyStatus);
                    ostr.write(id);

                    if (facet.empty())
                    {
                        ostr.write(static_cast<string*>(nullptr), static_cast<string*>(nullptr));
                    }
                    else
                    {
                        ostr.write(&facet, &facet + 1);
                    }
                    ostr.write(operation, false);
                }
                // dispatchExceptionMessage remains empty: RequestFailedException does not carry a message and we
                // don't include this message in OutgoingResponse.
            }
            else
            {
                dispatchExceptionMessage = ex.what();
                // And marshal this exception after the last catch block.
            }
        }
        catch (const LocalException& ex)
        {
            exceptionId = ex.ice_id();
            exceptionDetails = toString(ex);
            dispatchExceptionMessage = createDispatchExceptionMessage(exceptionId, ex.what());
            replyStatus = ReplyStatus::UnknownLocalException;
        }
        catch (const std::exception& ex)
        {
            exceptionId = demangle(typeid(ex).name());
            ostringstream str;
            str << "c++ exception: " << ex.what();
            exceptionDetails = str.str();
            dispatchExceptionMessage = createDispatchExceptionMessage(exceptionId, ex.what());
            replyStatus = ReplyStatus::UnknownException;
        }
        catch (...)
        {
            exceptionId = "unknown";
            exceptionDetails = "c++ exception: unknown";
            dispatchExceptionMessage = createDispatchExceptionMessage(exceptionId, "c++ exception");
            replyStatus = ReplyStatus::UnknownException;
        }

        if (current.requestId != 0 && replyStatus > ReplyStatus::OperationNotExist)
        {
            // We can't use the generated code to marshal a possibly unknown reply status.
            ostr.write(static_cast<uint8_t>(replyStatus));
            ostr.write(dispatchExceptionMessage);
        }

        return OutgoingResponse{
            replyStatus,
            std::move(exceptionId),
            std::move(exceptionDetails),
            std::move(ostr),
            current};
    }
} // anonymous namespace

OutgoingResponse::OutgoingResponse(
    ReplyStatus replyStatus,
    string exceptionId,
    string exceptionDetails,
    OutputStream outputStream,
    const Current& current) noexcept
    : _current(current),
      _exceptionId(std::move(exceptionId)),
      _exceptionDetails(std::move(exceptionDetails)),
      _outputStream(std::move(outputStream)),
      _replyStatus(replyStatus)
{
}

int32_t
OutgoingResponse::size() const noexcept
{
    return _current.get().requestId == 0 ? 0 : static_cast<int32_t>(_outputStream.b.size() - headerSize - 4);
}

OutgoingResponse
Ice::makeOutgoingResponse(
    const std::function<void(OutputStream*)>& marshal,
    const Current& current,
    std::optional<FormatType> format) noexcept
{
    assert(marshal);
    OutputStream ostr{current.adapter->getCommunicator(), Ice::currentProtocolEncoding};
    if (current.requestId != 0)
    {
        try
        {
            ostr.writeBlob(replyHdr, sizeof(replyHdr));
            ostr.write(current.requestId);
            ostr.write(ReplyStatus::Ok);
            ostr.startEncapsulation(current.encoding, format);
            marshal(&ostr);
            ostr.endEncapsulation();
            return OutgoingResponse{std::move(ostr), current};
        }
        catch (...)
        {
            return makeOutgoingResponse(current_exception(), current);
        }
    }
    else
    {
        // A oneway request cannot have a return or out params.
        assert(0);
        return OutgoingResponse{std::move(ostr), current};
    }
}

OutgoingResponse
Ice::makeEmptyOutgoingResponse(const Current& current) noexcept
{
    OutputStream ostr(current.adapter->getCommunicator(), Ice::currentProtocolEncoding);
    if (current.requestId != 0)
    {
        try
        {
            ostr.writeBlob(replyHdr, sizeof(replyHdr));
            ostr.write(current.requestId);
            ostr.write(ReplyStatus::Ok);
            ostr.writeEmptyEncapsulation(current.encoding);
        }
        catch (...)
        {
            return makeOutgoingResponse(current_exception(), current);
        }
    }
    return OutgoingResponse{std::move(ostr), current};
}

OutgoingResponse
Ice::makeOutgoingResponse(bool ok, pair<const byte*, const byte*> encapsulation, const Current& current) noexcept
{
    OutputStream ostr{current.adapter->getCommunicator(), Ice::currentProtocolEncoding};
    if (current.requestId != 0)
    {
        try
        {
            ostr.writeBlob(replyHdr, sizeof(replyHdr));
            ostr.write(current.requestId);
            ostr.write(ok ? ReplyStatus::Ok : ReplyStatus::UserException);
            ptrdiff_t size = encapsulation.second - encapsulation.first;
            assert(size >= 0);
            if (size > 0)
            {
                ostr.writeEncapsulation(encapsulation.first, static_cast<int32_t>(size));
            }
            else
            {
                ostr.writeEmptyEncapsulation(current.encoding);
            }
        }
        catch (...)
        {
            return makeOutgoingResponse(current_exception(), current);
        }
    }
    return OutgoingResponse{ok ? ReplyStatus::Ok : ReplyStatus::UserException, "", "", std::move(ostr), current};
}

OutgoingResponse
Ice::makeOutgoingResponse(std::exception_ptr exc, const Current& current) noexcept
{
    try
    {
        return makeOutgoingResponseCore(exc, current);
    }
    catch (...)
    {
        // This could throw again, but then it's either a bug or we can't allocate anything.
        return makeOutgoingResponseCore(current_exception(), current);
    }
}
