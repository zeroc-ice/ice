// Copyright (c) ZeroC, Inc.

#nullable enable

using System.Diagnostics;
using System.Text;

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

                case ReplyStatus.UnknownException:
                case ReplyStatus.UnknownUserException:
                case ReplyStatus.UnknownLocalException:
                    logDispatchFailed(response.exceptionDetails, request.current); // always log when middleware installed
                    break;

                default:
                    if (_traceLevel > 0 || _warningLevel > 1)
                    {
                        logDispatchFailed(response.exceptionDetails, request.current);
                    }
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
        catch (UnknownException ex)
        {
            logDispatchFailed(ex.ToString(), request.current); // always log when middleware installed
            throw;
        }
        catch (DispatchException ex)
        {
            if (_traceLevel > 0 || _warningLevel > 1)
            {
                logDispatchFailed(ex.ToString(), request.current);
            }
            throw;
        }
        catch (System.Exception ex)
        {
            logDispatchFailed(ex.ToString(), request.current);
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

    private void logDispatch(ReplyStatus replyStatus, Current current)
    {
        var sb = new StringBuilder();
        sb.Append("dispatch of ");
        sb.Append(current.operation);
        sb.Append(" to ");
        printTarget(sb, current);
        sb.Append(" returned a response with reply status ");
        sb.Append(replyStatus);

        _logger.trace(_traceCat, sb.ToString());
    }

    private void logDispatchFailed(string? exceptionDetails, Current current)
    {
        var sb = new StringBuilder();
        sb.Append("failed to dispatch ");
        sb.Append(current.operation);
        sb.Append(" to ");
        printTarget(sb, current);

        if (exceptionDetails is not null)
        {
            sb.Append(":\n");
            sb.Append(exceptionDetails);
        }

        _logger.warning(sb.ToString());
    }

    private void printTarget(StringBuilder sb, Current current)
    {
        sb.Append(Ice.Util.identityToString(current.id, _toStringMode));
        if (current.facet.Length > 0)
        {
            sb.Append(" -f ");
            sb.Append(Ice.UtilInternal.StringUtil.escapeString(current.facet, "", _toStringMode));
        }

        sb.Append(" over ");

        if (current.con is not null)
        {
            ConnectionInfo? connInfo = null;
            try
            {
                connInfo = current.con.getInfo();
                while (connInfo.underlying is not null)
                {
                    connInfo = connInfo.underlying;
                }
            }
            catch
            {
                // Thrown by getInfo() when the connection is closed.
            }

            if (connInfo is IPConnectionInfo ipConnInfo)
            {
                sb.Append(ipConnInfo.localAddress);
                sb.Append(':');
                sb.Append(ipConnInfo.localPort);
                sb.Append("<->");
                sb.Append(ipConnInfo.remoteAddress);
                sb.Append(':');
                sb.Append(ipConnInfo.remotePort);
            }
            else
            {
                // Connection.ToString() returns a multiline string, so we just use type here for bt and similar.
                sb.Append(current.con.type());
            }
        }
        else
        {
            sb.Append("colloc");
        }
    }
}
