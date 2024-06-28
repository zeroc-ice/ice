//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/OutgoingResponse.h"
#include "Ice/LocalExceptions.h"
#include "Ice/ObjectAdapter.h"
#include "Ice/UserException.h"
#include "Protocol.h"
#include "RequestFailedMessage.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace IceInternal
{
    extern bool printStackTraces;
}

namespace
{
    // The "core" implementation of makeOutgoingResponse for exceptions. Note that it can throw an exception.
    OutgoingResponse makeOutgoingResponseCore(std::exception_ptr exc, const Current& current)
    {
        assert(exc);
        OutputStream ostr(current.adapter->getCommunicator(), Ice::currentProtocolEncoding);

        if (current.requestId != 0)
        {
            ostr.writeBlob(replyHdr, sizeof(replyHdr));
            ostr.write(current.requestId);
        }
        ReplyStatus replyStatus;
        string exceptionId;
        string exceptionMessage;

        try
        {
            rethrow_exception(exc);
        }
        catch (RequestFailedException& rfe)
        {
            exceptionId = rfe.ice_id();

            if (dynamic_cast<ObjectNotExistException*>(&rfe))
            {
                replyStatus = ReplyStatus::ObjectNotExist;
            }
            else if (dynamic_cast<FacetNotExistException*>(&rfe))
            {
                replyStatus = ReplyStatus::FacetNotExist;
            }
            else if (dynamic_cast<OperationNotExistException*>(&rfe))
            {
                replyStatus = ReplyStatus::OperationNotExist;
            }
            else
            {
                assert(false);
                // Need to set replyStatus otherwise the compiler complains about uninitialized variable.
                replyStatus = ReplyStatus::ObjectNotExist;
            }

            Identity id = rfe.id();
            string facet = rfe.facet();
            string operation = rfe.operation();
            if (id.name.empty())
            {
                id = current.id;
                facet = current.facet;
            }
            if (operation.empty())
            {
                operation = current.operation;
            }

            // +7 to slice-off "::Ice::".
            exceptionMessage = createRequestFailedMessage(rfe.ice_id() + 7, id, facet, operation);

            if (current.requestId != 0)
            {
                ostr.write(static_cast<uint8_t>(replyStatus));
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
        }
        catch (const UserException& ex)
        {
            exceptionId = ex.ice_id();
            exceptionMessage = ex.what();

            replyStatus = ReplyStatus::UserException;

            if (current.requestId != 0)
            {
                ostr.write(static_cast<uint8_t>(replyStatus));
                ostr.startEncapsulation(current.encoding, FormatType::SlicedFormat);
                ostr.write(ex);
                ostr.endEncapsulation();
            }
        }
        catch (const UnknownLocalException& ex)
        {
            exceptionId = ex.ice_id();
            replyStatus = ReplyStatus::UnknownLocalException;
            exceptionMessage = ex.what();
        }
        catch (const UnknownUserException& ex)
        {
            exceptionId = ex.ice_id();
            replyStatus = ReplyStatus::UnknownUserException;
            exceptionMessage = ex.what();
        }
        catch (const UnknownException& ex)
        {
            exceptionId = ex.ice_id();
            replyStatus = ReplyStatus::UnknownException;
            exceptionMessage = ex.what();
        }
        catch (const LocalException& ex)
        {
            exceptionId = ex.ice_id();
            replyStatus = ReplyStatus::UnknownLocalException;
            ostringstream str;
            str << ex; // this includes more details than ex.what()
            if (IceInternal::printStackTraces)
            {
                str << '\n' << ex.ice_stackTrace();
            }
            exceptionMessage = str.str();
        }
        catch (const Exception& ex)
        {
            exceptionId = ex.ice_id();
            replyStatus = ReplyStatus::UnknownException;
            ostringstream str;
            str << ex; // this includes more details than ex.what()
            if (IceInternal::printStackTraces)
            {
                str << '\n' << ex.ice_stackTrace();
            }
            exceptionMessage = str.str();
        }
        catch (const std::exception& ex)
        {
            replyStatus = ReplyStatus::UnknownException;
            exceptionId = typeid(ex).name(); // can be a mangled name with some compilers
            ostringstream str;
            str << "c++ exception: " << ex.what();
            exceptionMessage = str.str();
        }
        catch (...)
        {
            replyStatus = ReplyStatus::UnknownException;
            exceptionId = "unknown";
            exceptionMessage = "c++ exception: unknown";
        }

        if ((current.requestId != 0) &&
            (replyStatus == ReplyStatus::UnknownUserException || replyStatus == ReplyStatus::UnknownLocalException ||
             replyStatus == ReplyStatus::UnknownException))
        {
            ostr.write(static_cast<uint8_t>(replyStatus));
            ostr.write(exceptionMessage);
        }

        return OutgoingResponse{
            replyStatus,
            std::move(exceptionId),
            std::move(exceptionMessage),
            std::move(ostr),
            current};
    }
} // anonymous namespace

OutgoingResponse::OutgoingResponse(
    ReplyStatus replyStatus,
    string exceptionId,
    string exceptionMessage,
    OutputStream outputStream,
    const Current& current) noexcept
    : _current(current),
      _exceptionId(std::move(exceptionId)),
      _exceptionMessage(std::move(exceptionMessage)),
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
    std::function<void(OutputStream*)> marshal,
    const Current& current,
    FormatType format) noexcept
{
    assert(marshal);
    OutputStream ostr(current.adapter->getCommunicator(), Ice::currentProtocolEncoding);
    if (current.requestId != 0)
    {
        try
        {
            ostr.writeBlob(replyHdr, sizeof(replyHdr));
            ostr.write(current.requestId);
            ostr.write(static_cast<uint8_t>(ReplyStatus::Ok));
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
            ostr.write(static_cast<uint8_t>(ReplyStatus::Ok));
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
    OutputStream ostr(current.adapter->getCommunicator(), Ice::currentProtocolEncoding);
    if (current.requestId != 0)
    {
        try
        {
            ostr.writeBlob(replyHdr, sizeof(replyHdr));
            ostr.write(current.requestId);
            ostr.write(static_cast<uint8_t>(ok ? ReplyStatus::Ok : ReplyStatus::UserException));
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
    return OutgoingResponse{std::move(ostr), current};
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
