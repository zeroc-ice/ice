// Copyright (c) ZeroC, Inc.

#nullable enable

using System.Globalization;

namespace Ice.Internal;

/// <summary>A middleware that logs warnings for failed dispatches.</summary>
internal sealed class LoggerMiddleware : Object
{
    private readonly Object _next;
    private readonly Logger _logger;
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
                    // no warning
                    break;

                case ReplyStatus.ObjectNotExist:
                case ReplyStatus.FacetNotExist:
                case ReplyStatus.OperationNotExist:
                    if (_warningLevel > 1)
                    {
                        warning(response.exceptionDetails, request.current);
                    }
                    break;

                default:
                    warning(response.exceptionDetails, request.current);
                    break;
            }
            return response;
        }
        catch (UserException)
        {
            // No warning
            throw;
        }
        catch (RequestFailedException ex)
        {
            if (_warningLevel > 1)
            {
                warning(ex.ToString(), request.current);
            }
            throw;
        }
        catch (System.Exception ex)
        {
            warning(ex.ToString(), request.current);
            throw;
        }
    }

    internal LoggerMiddleware(Object next, Logger logger, int warningLevel, ToStringMode toStringMode)
    {
        _next = next;
        _logger = logger;
        _warningLevel = warningLevel;
        _toStringMode = toStringMode;
    }

    private void warning(string? exceptionDetails, Current current)
    {
        var sb = new System.Text.StringBuilder();
        sb.AppendLine("dispatch exception:");

        sb.Append("identity: ");
        sb.AppendLine(Ice.Util.identityToString(current.id, _toStringMode));

        sb.Append("facet: ");
        sb.AppendLine(Ice.UtilInternal.StringUtil.escapeString(current.facet, "", _toStringMode));

        sb.Append("operation: ");
        sb.AppendLine(current.operation);

        if (current.con is not null)
        {
            try
            {
                for (ConnectionInfo? p = current.con.getInfo(); p is not null; p = p.underlying)
                {
                    if (p is IPConnectionInfo ipInfo)
                    {
                        sb.Append("remote host: ");
                        sb.Append(ipInfo.remoteAddress);
                        sb.Append(" remote port: ");
                        sb.AppendLine(ipInfo.remotePort.ToString(CultureInfo.InvariantCulture));
                        break;
                    }
                }
            }
            catch (Ice.LocalException)
            {
            }
        }

        if (exceptionDetails is not null)
        {
            sb.AppendLine();
            sb.Append(exceptionDetails);
        }
        _logger.warning(sb.ToString());
    }
}
