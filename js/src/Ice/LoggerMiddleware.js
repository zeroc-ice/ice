// Copyright (c) ZeroC, Inc.

import { UserException } from "./UserException.js";
import { DispatchException } from "./LocalExceptions.js";
import { UnknownException } from "./LocalExceptions.js";
import { IPConnectionInfo } from "./Connection.js";
import { identityToString } from "./IdentityToString.js";
import { StringUtil } from "./StringUtil.js";
import { Ice as Ice_ReplyStatus } from "./ReplyStatus.js";
const { ReplyStatus } = Ice_ReplyStatus;

export class LoggerMiddleware {
    async dispatch(request) {
        try {
            const response = await this._next.dispatch(request);
            switch (response.replyStatus) {
                case ReplyStatus.Ok:
                case ReplyStatus.UserException:
                    if (this._traceLevel > 0) {
                        this.logDispatch(response.replyStatus, request.current);
                    }
                    break;

                case ReplyStatus.UnknownException:
                case ReplyStatus.UnknownUserException:
                case ReplyStatus.UnknownLocalException:
                    // Always log when middleware installed.
                    this.logDispatchFailed(response.exceptionDetails, request.current);
                    break;

                default:
                    if (this._traceLevel > 0 || this._warningLevel > 1) {
                        this.logDispatchFailed(response.exceptionDetails, request.current);
                    }
                    break;
            }
            return response;
        } catch (ex) {
            if (ex instanceof UserException) {
                if (this._traceLevel > 0) {
                    logDispatch(ReplyStatus.UserException, request.current);
                }
            } else if (ex instanceof UnknownException) {
                // Always log when middleware installed.
                // The stack includes the class name and error message.
                this.logDispatchFailed(ex.stack, request.current);
            } else if (ex instanceof DispatchException) {
                if (this._traceLevel > 0 || this._warningLevel > 1) {
                    this.logDispatchFailed(ex.stack, request.current);
                }
            } else {
                this.logDispatchFailed(ex.stack, request.current);
            }
            throw ex;
        }
    }

    constructor(next, logger, traceLevel, traceCat, warningLevel, toStringMode) {
        this._next = next;
        this._logger = logger;
        this._traceLevel = traceLevel;
        this._traceCat = traceCat;
        this._warningLevel = warningLevel;
        this._toStringMode = toStringMode;

        DEV: console.assert(traceLevel > 0 || warningLevel > 0);
    }

    logDispatch(replyStatus, current) {
        const output = [];
        output.push("dispatch of ");
        output.push(current.operation);
        output.push(" to ");
        printTarget(output, this._toStringMode, current);
        output.push(" returned a response with reply status ");
        output.push(replyStatus);
        this._logger.trace(this._traceCat, output.join(""));
    }

    logDispatchFailed(exceptionDetails, current) {
        const output = [];
        output.push("failed to dispatch ");
        output.push(current.operation);
        output.push(" to ");
        printTarget(output, this._toStringMode, current);

        if (exceptionDetails !== null) {
            output.push("\n");
            output.push(exceptionDetails);
        }
        this._logger.warning(output.join(""));
    }
}

function printTarget(output, toStringMode, current) {
    output.push("'");
    output.push(identityToString(current.id, toStringMode));
    output.push("'");
    if (current.facet !== "") {
        output.push(" -f '");
        output.push(StringUtil.escapeString(current.facet, "", toStringMode));
        output.push("'");
    }
    output.push(" over ");

    DEV: console.assert(current.con !== null, "current.con is null"); // no colloc in JS

    let connInfo = null;
    try {
        connInfo = current.con.getInfo();
        while (connInfo.underlying !== null) {
            connInfo = connInfo.underlying;
        }
    } catch (ex) {
        // Thrown by getInfo() when the connection is closed.
    }

    if (connInfo instanceof IPConnectionInfo) {
        output.push(connInfo.localAddress);
        output.push(":");
        output.push(connInfo.localPort);
        output.push("<->");
        output.push(connInfo.remoteAddress);
        output.push(":");
        output.push(connInfo.remotePort);
    } else {
        // Connection.toString() returns a multiline string, so we just use type here for bt and similar.
        output.push(current.con.type());
    }
}
