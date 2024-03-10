//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/ObjectAdapter.h"
#include "Ice/OutgoingResponse.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace IceUtilInternal
{
    extern bool printStackTraces;
}

namespace
{
    // The "core" implementation of makeOutgoingResponse for exceptions. Note that it can throw an exception.
    OutgoingResponse makeOutgoingResponseCore(std::exception_ptr exc, const Current& current)
    {
        OutputStream ostr(current.adapter->getCommunicator(), Ice::currentProtocolEncoding);

        if (current.requestId != 0)
        {
            ostr.writeBlob(replyHdr, sizeof(replyHdr));
            ostr.write(current.requestId);
        }
        ReplyStatus replyStatus;
        string exceptionId;
        string errorMessage;

        try
        {
            rethrow_exception(exc);
        }
        catch (RequestFailedException& rfe)
        {
            exceptionId = rfe.ice_id();
            errorMessage = rfe.what();

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

            if (rfe.id.name.empty())
            {
                rfe.id = current.id;
            }

            if (rfe.facet.empty() && !current.facet.empty())
            {
                rfe.facet = current.facet;
            }

            if (rfe.operation.empty() && !current.operation.empty())
            {
                rfe.operation = current.operation;
            }

            if (current.requestId != 0)
            {
                ostr.write(static_cast<uint8_t>(replyStatus));
                ostr.write(rfe.id);

                if (rfe.facet.empty())
                {
                    ostr.write(static_cast<string*>(nullptr), static_cast<string*>(nullptr));
                }
                else
                {
                    ostr.write(&rfe.facet, &rfe.facet + 1);
                }

                ostr.write(rfe.operation, false);
            }
        }
        catch (const UserException& ex)
        {
            exceptionId = ex.ice_id();
            errorMessage = ex.what();

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
            errorMessage = ex.unknown;
        }
        catch (const UnknownUserException& ex)
        {
            exceptionId = ex.ice_id();
            replyStatus = ReplyStatus::UnknownUserException;
            errorMessage = ex.unknown;
        }
        catch (const UnknownException& ex)
        {
            exceptionId = ex.ice_id();
            replyStatus = ReplyStatus::UnknownException;
            errorMessage = ex.unknown;
        }
        catch (const LocalException& ex)
        {
            exceptionId = ex.ice_id();
            replyStatus = ReplyStatus::UnknownLocalException;
            ostringstream str;
            str << ex;
            if (IceUtilInternal::printStackTraces)
            {
                str << '\n' << ex.ice_stackTrace();
            }
            errorMessage = str.str();
        }
        catch (const Exception& ex)
        {
            exceptionId = ex.ice_id();
            replyStatus = ReplyStatus::UnknownException;
            ostringstream str;
            str << ex;
            if (IceUtilInternal::printStackTraces)
            {
                str << '\n' << ex.ice_stackTrace();
            }
            errorMessage = str.str();
        }
        catch (const std::exception& ex)
        {
            replyStatus = ReplyStatus::UnknownException;
            exceptionId = ex.what();
            ostringstream str;
            str << "c++ exception: " << exceptionId;
            errorMessage = str.str();
        }
        catch (...)
        {
            replyStatus = ReplyStatus::UnknownException;
            exceptionId = "unknown";
            errorMessage = "c++ exception: unknown";
        }

        if ((current.requestId != 0) &&
            (replyStatus == ReplyStatus::UnknownUserException || replyStatus == ReplyStatus::UnknownLocalException ||
             replyStatus == ReplyStatus::UnknownException))
        {
            ostr.write(static_cast<uint8_t>(replyStatus));
            ostr.write(errorMessage);
        }

        return OutgoingResponse{replyStatus, std::move(exceptionId), std::move(errorMessage), ostr, current};
    }
} // anonymous namespace

OutgoingResponse::OutgoingResponse(
    ReplyStatus replyStatus,
    string exceptionId,
    string errorMessage,
    OutputStream& outputStream,
    const Current& current) noexcept
    : _current(current),
      _exceptionId(std::move(exceptionId)),
      _errorMessage(std::move(errorMessage)),
      _replyStatus(replyStatus)
{
    _outputStream.swap(outputStream);
}

OutgoingResponse::OutgoingResponse(OutgoingResponse&& other) noexcept
    : _current(std::move(other._current)),
      _exceptionId(std::move(other._exceptionId)),
      _errorMessage(std::move(other._errorMessage)),
      _replyStatus(other._replyStatus)
{
    _outputStream.swap(other._outputStream);
}

OutgoingResponse&
OutgoingResponse::operator=(OutgoingResponse&& other) noexcept
{
    _current = std::move(other._current);
    _replyStatus = other._replyStatus;
    _exceptionId = std::move(other._exceptionId);
    _errorMessage = std::move(other._errorMessage);
    _outputStream.swap(other._outputStream);
    return *this;
}

int32_t
OutgoingResponse::size() const noexcept
{
    return _current.get().requestId == 0 ? 0 : static_cast<int32_t>(_outputStream.b.size() - sizeof(replyHdr));
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
            return OutgoingResponse{ostr, current};
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
    }
    return OutgoingResponse{ostr, current};
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
    return OutgoingResponse{ostr, current};
}

OutgoingResponse
Ice::makeOutgoingResponse(
    bool ok,
    const pair<const uint8_t*, const uint8_t*>& encapsulation,
    const Current& current) noexcept
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
    return OutgoingResponse{ostr, current};
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
