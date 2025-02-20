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

                                var replyStatus = ReplyStatus.valueOf(response.replyStatus);
                                if (replyStatus != null) {
                                    switch (replyStatus) {
                                        case Ok:
                                        case UserException:
                                            if (_traceLevel > 0) {
                                                logDispatch(replyStatus, request.current);
                                            }
                                            break;

                                        case UnknownException:
                                        case UnknownUserException:
                                        case UnknownLocalException:
                                            // always log when middleware installed
                                            logDispatchFailed(
                                                    response.exceptionDetails, request.current);
                                            break;

                                        default:
                                            if (_traceLevel > 0 || _warningLevel > 1) {
                                                logDispatchFailed(
                                                        response.exceptionDetails, request.current);
                                            }
                                            break;
                                    }
                                } else {
                                    // Unknown reply status, like default case above.
                                    if (_traceLevel > 0 || _warningLevel > 1) {
                                        logDispatchFailed(
                                                response.exceptionDetails, request.current);
                                    }
                                }
                                return response;
                            });
        } catch (UserException ex) {
            if (_traceLevel > 0) {
                logDispatch(ReplyStatus.UserException, request.current);
            }
            throw ex;
        } catch (UnknownException ex) {
            logDispatchFailed(
                    ex.toString(), request.current); // always log when middleware installed
            throw ex;
        } catch (DispatchException ex) {
            if (_traceLevel > 0 || _warningLevel > 1) {
                logDispatchFailed(ex.toString(), request.current);
            }
            throw ex;
        } catch (RuntimeException | Error ex) {
            logDispatchFailed(ex.toString(), request.current);
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

    private void logDispatchFailed(String exceptionDetails, Current current) {
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
        out.print(" over ");

        if (current.con != null) {
            ConnectionInfo connInfo = null;
            try {
                connInfo = current.con.getInfo();
                while (connInfo.underlying != null) {
                    connInfo = connInfo.underlying;
                }
            } catch (Exception e) {
                // Thrown by getInfo() when the connection is closed.
            }

            if (connInfo instanceof IPConnectionInfo ipConnInfo) {
                out.print(ipConnInfo.localAddress);
                out.print(":" + ipConnInfo.localPort);
                out.print("<->");
                out.print(ipConnInfo.remoteAddress);
                out.print(":" + ipConnInfo.remotePort);
            } else {
                // Connection.toString() returns a multiline string, so we just use type() here
                // for bt and similar.
                out.print(current.con.type());
            }
        } else {
            out.print("colloc");
        }
    }
}
