// Copyright (c) ZeroC, Inc.

#nullable enable

using System.Diagnostics;

namespace Ice.Internal;

/// <summary>A middleware that logs warnings for failed dispatches.</summary>
internal sealed class LoggerMiddleware : Object
{
    private readonly Object _next;
    private readonly Logger _logger;

    private readonly string _traceCat;
    private readonly int _traceLevel;
    private readonly int _warningLevel;
    private readonly ToStringMode _toStringMode;

    public async ValueTask<OutgoingResponse> dispatchAsync(IncomingRequest request)
    {
        try
        {
            OutgoingResponse response = await _next.dispatchAsync(request).ConfigureAwait(false);
            switch (response.replyStatus)
            {
                case ReplyStatus.Ok:
                case ReplyStatus.UserException:
                    if (_traceLevel > 0)
                    {
                        logDispatch(response.replyStatus, request.current);
                    }
                    break;

                case ReplyStatus.ObjectNotExist:
                case ReplyStatus.FacetNotExist:
                case ReplyStatus.OperationNotExist:
                    if (_traceLevel > 0 || _warningLevel > 1)
                    {
                        logDispatchException(response.exceptionDetails, request.current);
                    }
                    break;

                default:
                    logDispatchException(response.exceptionDetails, request.current);
                    break;
            }
            return response;
        }
        catch (UserException)
        {
            if (_traceLevel > 0)
            {
                logDispatch(ReplyStatus.UserException, request.current);
            }
            throw;
        }
        catch (RequestFailedException ex)
        {
            if (_traceLevel > 0 || _warningLevel > 1)
            {
                logDispatchException(ex.ToString(), request.current);
            }
            throw;
        }
        catch (System.Exception ex)
        {
            logDispatchException(ex.ToString(), request.current);
            throw;
        }
    }

    internal LoggerMiddleware(Object next, Logger logger, int traceLevel, string traceCat, int warningLevel, ToStringMode toStringMode)
    {
        _next = next;
        _logger = logger;
        _traceLevel = traceLevel;
        _traceCat = traceCat;
        _warningLevel = warningLevel;
        _toStringMode = toStringMode;

        Debug.Assert(_traceLevel > 0 || _warningLevel > 0);
    }

    private void logDispatch(ReplyStatus replyStatus, Current current) =>
        _logger.trace(
            _traceCat,
            $"dispatch of {current.operation} to {getTarget(current)} returned a response with reply status {replyStatus}");

    private void logDispatchException(string? exceptionDetails, Current current)
    {
        if (exceptionDetails is null)
        {
            _logger.warning($"failed to dispatch {current.operation} to {getTarget(current)}");
        }
        else
        {
            _logger.warning(
                $"failed to dispatch {current.operation} to {getTarget(current)}:\n{exceptionDetails}");
        }
    }

    private string getTarget(Current current)
    {
        string target = Ice.Util.identityToString(current.id, _toStringMode);
        if (current.facet.Length > 0)
        {
            target = $"{target} -f {current.facet}";
        }

        if (current.con is not null)
        {
            try
            {
                for (ConnectionInfo? p = current.con.getInfo(); p is not null; p = p.underlying)
                {
                    if (p is IPConnectionInfo ipInfo)
                    {
                        target = $"{target} over {ipInfo.localAddress}:{ipInfo.localPort}<->{ipInfo.remoteAddress}:{ipInfo.remotePort}";
                        break;
                    }
                }
            }
            catch (LocalException)
            {
            }
        }
        return target;
    }
}
