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
                    // always log when middleware installed
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
                // always log when middleware installed
                this.logDispatchFailed(`${ex}\n${ex.stack}`, request.current);
            } else if (ex instanceof DispatchException) {
                if (this._traceLevel > 0 || this._warningLevel > 1) {
                    this.logDispatchFailed(`${ex}\n${ex.stack}`, request.current);
                }
            } else {
                this.logDispatchFailed(`${ex}\n${ex.stack}`, request.current);
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
        let output = `dispatch of ${current.operation} to `;
        this.printTarget(output, current);
        output += ` returned a response with reply status ${replyStatus.name}`;
        this._logger.trace(this._traceCat, output);
    }

    logDispatchFailed(exceptionMessage, current) {
        let output = `failed to dispatch ${current.operation} to `;
        this.printTarget(output, current);

        if (exceptionMessage !== null) {
            output += `:\n${exceptionMessage}`;
        }
        this._logger.warning(output);
    }

    printTarget(output, current) {
        output += `${identityToString(current.id, this._toStringMode)}`;
        if (current.facet !== "") {
            output += ` -f ${StringUtil.escapeString(current.facet, "", this._toStringMode)}`;
        }
        output += " over ";

        if (current.con !== null) {
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
                output += `${connInfo.localAddress}:${connInfo.localPort}`;
                output += "<->";
                output += `${connInfo.remoteAddress}:${connInfo.remotePort}`;
            } else {
                // Connection.toString() returns a multiline string, so we just use type here for bt and similar.
                output += `${current.con.type()}`;
            }
        } else {
            output += "colloc";
        }
    }
}
