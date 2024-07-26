// Copyright (c) ZeroC, Inc.

package com.zeroc.IceInternal;

import com.zeroc.Ice.ConnectionInfo;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.IPConnectionInfo;
import com.zeroc.Ice.IncomingRequest;
import com.zeroc.Ice.LocalException;
import com.zeroc.Ice.Logger;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.OutgoingResponse;
import com.zeroc.Ice.RequestFailedException;
import com.zeroc.Ice.ToStringMode;
import com.zeroc.Ice.UserException;
import com.zeroc.Ice.Util;
import com.zeroc.IceUtilInternal.StringUtil;
import java.util.concurrent.CompletionStage;

public final class LoggerMiddleware implements Object {
  private final Object _next;
  private final Logger _logger;
  private final int _warningLevel;
  private final ToStringMode _toStringMode;

  public LoggerMiddleware(Object next, Logger logger, int warningLevel, ToStringMode toStringMode) {
    _next = next;
    _logger = logger;
    _warningLevel = warningLevel;
    _toStringMode = toStringMode;
  }

  @Override
  public CompletionStage<OutgoingResponse> dispatch(IncomingRequest request) throws UserException {
    try {
      return _next
          .dispatch(request)
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
    } catch (RuntimeException | java.lang.Error ex) {
      warning(ex.toString(), request.current);
      throw ex;
    }
  }

  private void warning(String exceptionDetails, Current current) {
    java.io.StringWriter sw = new java.io.StringWriter();
    java.io.PrintWriter pw = new java.io.PrintWriter(sw);
    com.zeroc.IceUtilInternal.OutputBase out = new com.zeroc.IceUtilInternal.OutputBase(pw);
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
      if (exceptionDetails != null) {
        out.print("\n");
        out.print(exceptionDetails);
      }
      _logger.warning(sw.toString());
    }
  }
}
