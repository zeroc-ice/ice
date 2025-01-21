// Copyright (c) ZeroC, Inc.

#include "LoggerMiddleware.h"
#include "Ice/StringUtil.h"
#include "Ice/UserException.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

LoggerMiddleware::LoggerMiddleware(Ice::ObjectPtr next, LoggerPtr logger, int warningLevel, ToStringMode toStringMode)
    : _next(std::move(next)),
      _logger(std::move(logger)),
      _warningLevel(warningLevel),
      _toStringMode(toStringMode)
{
    assert(_next);
    assert(_logger);
    assert(_warningLevel > 0);
}

void
LoggerMiddleware::dispatch(Ice::IncomingRequest& request, function<void(OutgoingResponse)> sendResponse)
{
    try
    {
        _next->dispatch(
            request,
            [sendResponse = std::move(sendResponse), self = shared_from_this()](OutgoingResponse response)
            {
                switch (response.replyStatus())
                {
                    case ReplyStatus::Ok:
                    case ReplyStatus::UserException:
                        // no warning
                        break;
                    case ReplyStatus::ObjectNotExist:
                    case ReplyStatus::FacetNotExist:
                    case ReplyStatus::OperationNotExist:
                        if (self->_warningLevel > 1)
                        {
                            self->warning(response.exceptionDetails(), response.current());
                        }
                        break;

                    default:
                        self->warning(response.exceptionDetails(), response.current());
                        break;
                }
                sendResponse(std::move(response));
            });
    }
    catch (const UserException&)
    {
        // No warning.
        throw;
    }
    catch (const RequestFailedException& ex)
    {
        if (_warningLevel > 1)
        {
            warning(ex, request.current());
        }
        throw;
    }
    catch (const Ice::Exception& ex)
    {
        warning(ex, request.current());
        throw;
    }
    catch (const std::exception& ex)
    {
        warning(ex.what(), request.current());
        throw;
    }
    catch (...)
    {
        warning("c++ exception", request.current());
        throw;
    }
}

void
LoggerMiddleware::warning(const Exception& ex, const Current& current) const noexcept
{
    Warning out(_logger);
    out << "dispatch exception: " << ex;
    warning(out, current);
}

void
LoggerMiddleware::warning(const string& exceptionDetails, const Current& current) const noexcept
{
    Warning out(_logger);
    out << "dispatch exception: " << exceptionDetails;
    warning(out, current);
}

void
LoggerMiddleware::warning(Warning& out, const Current& current) const noexcept
{
    out << "\nidentity: " << identityToString(current.id, _toStringMode);
    out << "\nfacet: " << escapeString(current.facet, "", _toStringMode);
    out << "\noperation: " << current.operation;

    if (current.con)
    {
        for (Ice::ConnectionInfoPtr connInfo = current.con->getInfo(); connInfo; connInfo = connInfo->underlying)
        {
            Ice::IPConnectionInfoPtr ipConnInfo = dynamic_pointer_cast<Ice::IPConnectionInfo>(connInfo);
            if (ipConnInfo)
            {
                out << "\nremote host: " << ipConnInfo->remoteAddress << " remote port: " << ipConnInfo->remotePort;
                break;
            }
        }
    }
}
