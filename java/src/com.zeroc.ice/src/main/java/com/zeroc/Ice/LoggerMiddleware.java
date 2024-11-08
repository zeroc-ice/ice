// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.concurrent.CompletionStage;

final class LoggerMiddleware implements com.zeroc.Ice.Object {
    private final com.zeroc.Ice.Object _next;
    private final Logger _logger;
    private final int _warningLevel;
    private final ToStringMode _toStringMode;

    public LoggerMiddleware(
            com.zeroc.Ice.Object next, Logger logger, int warningLevel, ToStringMode toStringMode) {
        _next = next;
        _logger = logger;
        _warningLevel = warningLevel;
        _toStringMode = toStringMode;
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
                                        // no warning
                                        break;

                                    case ObjectNotExist:
                                    case FacetNotExist:
                                    case OperationNotExist:
                                        if (_warningLevel > 1) {
                                            warning(response.exceptionDetails, request.current);
                                        }
                                        break;

                                    default:
                                        warning(response.exceptionDetails, request.current);
                                        break;
                                }
                                return response;
                            });
        } catch (UserException ex) {
            // No warning
            throw ex;
        } catch (RequestFailedException ex) {
            if (_warningLevel > 1) {
                warning(ex.toString(), request.current);
            }
            throw ex;
        } catch (RuntimeException | Error ex) {
            warning(ex.toString(), request.current);
            throw ex;
        }
    }

    private void warning(String exceptionDetails, Current current) {
        var sw = new java.io.StringWriter();
        var pw = new java.io.PrintWriter(sw);
        var out = new OutputBase(pw);
        out.setUseTab(false);
        out.print("dispatch exception:");
        out.print("\nidentity: " + Util.identityToString(current.id, _toStringMode));
        out.print("\nfacet: " + StringUtil.escapeString(current.facet, "", _toStringMode));
        out.print("\noperation: " + current.operation);

        if (current.con != null) {
            try {
                for (ConnectionInfo connInfo = current.con.getInfo();
                        connInfo != null;
                        connInfo = connInfo.underlying) {
                    if (connInfo instanceof IPConnectionInfo ipConnInfo) {
                        out.print(
                                "\nremote host: "
                                        + ipConnInfo.remoteAddress
                                        + " remote port: "
                                        + ipConnInfo.remotePort);
                    }
                }
            } catch (LocalException exc) {
                // Ignore.
            }
        }

        if (exceptionDetails != null) {
            out.print("\n");
            out.print(exceptionDetails);
        }
        _logger.warning(sw.toString());
    }
}
