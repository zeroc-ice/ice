// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.concurrent.CompletionStage;

final class LoggerMiddleware implements com.zeroc.Ice.Object {
    private final com.zeroc.Ice.Object _next;
    private final Logger _logger;
    private final int _traceLevel;
    private final String _traceCat;
    private final int _warningLevel;
    private final ToStringMode _toStringMode;

    public LoggerMiddleware(
            com.zeroc.Ice.Object next,
            Logger logger,
            int traceLevel,
            String traceCat,
            int warningLevel,
            ToStringMode toStringMode) {
        _next = next;
        _logger = logger;
        _traceLevel = traceLevel;
        _traceCat = traceCat;
        _warningLevel = warningLevel;
        _toStringMode = toStringMode;

        assert _traceLevel > 0 || _warningLevel > 0;
    }

    @Override
    public CompletionStage<OutgoingResponse> dispatch(IncomingRequest request)
            throws UserException {
        try {
            return _next.dispatch(request)
                    .handle(
                            (response, exception) -> {
                                if (exception != null) {
                                    // Convert to response for further processing
                                    response = request.current.createOutgoingResponse(exception);
                                }
                                switch (response.replyStatus) {
                                    case Ok:
                                    case UserException:
                                        if (_traceLevel > 0) {
                                            logDispatch(response.replyStatus, request.current);
                                        }
                                        break;

                                    case ObjectNotExist:
                                    case FacetNotExist:
                                    case OperationNotExist:
                                        if (_traceLevel > 0 || _warningLevel > 1) {
                                            logDispatchException(
                                                    response.exceptionDetails, request.current);
                                        }
                                        break;

                                    default:
                                        logDispatchException(
                                                response.exceptionDetails, request.current);
                                        break;
                                }
                                return response;
                            });
        } catch (UserException ex) {
            if (_traceLevel > 0) {
                logDispatch(ReplyStatus.UserException, request.current);
            }
            throw ex;
        } catch (RequestFailedException ex) {
            if (_traceLevel > 0 || _warningLevel > 1) {
                logDispatchException(ex.toString(), request.current);
            }
            throw ex;
        } catch (RuntimeException | Error ex) {
            logDispatchException(ex.toString(), request.current);
            throw ex;
        }
    }

    private void logDispatch(ReplyStatus replyStatus, Current current) {
        var sw = new java.io.StringWriter();
        var pw = new java.io.PrintWriter(sw);
        var out = new OutputBase(pw);
        out.setUseTab(false);
        out.print("dispatch of ");
        out.print(current.operation);
        out.print(" to ");
        printTarget(out, current);
        out.print(" returned a response with reply status ");
        out.print(replyStatus.toString());

        _logger.trace(_traceCat, sw.toString());
    }

    private void logDispatchException(String exceptionDetails, Current current) {
        var sw = new java.io.StringWriter();
        var pw = new java.io.PrintWriter(sw);
        var out = new OutputBase(pw);
        out.setUseTab(false);
        out.print("failed to dispatch ");
        out.print(current.operation);
        out.print(" to ");
        printTarget(out, current);

        if (exceptionDetails != null) {
            out.print(":\n");
            out.print(exceptionDetails);
        }

        _logger.warning(sw.toString());
    }

    private void printTarget(OutputBase out, Current current) {
        out.print(Util.identityToString(current.id, _toStringMode));
        if (!current.facet.isEmpty()) {
            out.print(" -f ");
            out.print(StringUtil.escapeString(current.facet, "", _toStringMode));
        }
        if (current.con != null) {
            try {
                for (ConnectionInfo connInfo = current.con.getInfo();
                        connInfo != null;
                        connInfo = connInfo.underlying) {
                    if (connInfo instanceof IPConnectionInfo ipConnInfo) {
                        out.print(" over ");
                        out.print(ipConnInfo.localAddress);
                        out.print(":" + ipConnInfo.localPort);
                        out.print("<->");
                        out.print(ipConnInfo.remoteAddress);
                        out.print(":" + ipConnInfo.remotePort);
                    }
                }
            } catch (LocalException exc) {
                // Ignore.
            }
        }
    }
}
