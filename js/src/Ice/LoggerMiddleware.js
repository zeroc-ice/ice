//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { ReplyStatus } from "./ReplyStatus.js";
import { UserException } from "./UserException.js";
import { RequestFailedException } from "./LocalExceptions.js";
import { IPConnectionInfo } from "./Connection.js";
import { identityToString } from "./IdentityToString.js";
import { StringUtil } from "./StringUtil.js";

export class LoggerMiddleware {
    async dispatch(request) {
        try {
            const response = await this._next.dispatch(request);
            switch (response.replyStatus) {
                case ReplyStatus.Ok:
                case ReplyStatus.UserException:
                    // no warning
                    break;

                case ReplyStatus.ObjectNotExist:
                case ReplyStatus.FacetNotExist:
                case ReplyStatus.OperationNotExist:
                    if (_warningLevel > 1) {
                        this.warning(response.exceptionDetails, request.current);
                    }
                    break;

                default:
                    this.warning(response.exceptionDetails, request.current);
                    break;
            }
            return response;
        } catch (ex) {
            if (ex instanceof UserException) {
                // No warning
            } else if (ex instanceof RequestFailedException) {
                if (this._warningLevel > 1) {
                    this.warning(`${ex}\n${ex.stack}`, request.current);
                }
            } else {
                this.warning(`${ex}\n${ex.stack}`, request.current);
            }
            throw ex;
        }
    }

    constructor(next, logger, warningLevel, toStringMode) {
        this._next = next;
        this._logger = logger;
        this._warningLevel = warningLevel;
        this._toStringMode = toStringMode;
    }

    warning(exceptionMessage, current) {
        let output = "dispatch exception:";
        output += `\nidentity: ${identityToString(current.id, this._toStringMode)}`;
        output += `\nfacet: ${StringUtil.escapeString(current.facet, "", this._toStringMode)}`;
        output += `\noperation: ${current.operation}`;
        if (current.con !== null) {
            try {
                for (const p = current.con.getInfo(); p !== null; p = p.underlying) {
                    if (p instanceof IPConnectionInfo) {
                        output += `\nremote host: ${p.remoteAddress} remote port: ${p.remotePort}`;
                        break;
                    }
                }
            } catch (ex) {}
        }

        if (exceptionMessage !== null) {
            output += `\n${exceptionMessage}`;
        }
        this._logger.warning(output);
    }
}
