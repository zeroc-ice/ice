//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { Ice as Ice_Identity } from "./Identity.js";
const { Identity } = Ice_Identity;
import { StringUtil } from "./StringUtil.js";
import { Protocol } from "./Protocol.js";
import { Ice as Ice_OperationMode } from "./OperationMode.js";
import { Debug } from "./Debug.js";
import { identityToString } from "./IdentityToString.js";

import { Encoding_1_0, encodingVersionToString } from "./Protocol.js";
import { ReplyStatus } from "./ReplyStatus.js";
import { InputStream } from "./InputStream.js";

import { Ice as Ice_BuiltinSequences } from "./BuiltinSequences.js";
const { StringSeqHelper } = Ice_BuiltinSequences;

const OperationMode = Ice_OperationMode.OperationMode;

function printIdentityFacetOperation(s, inputStream) {
    const toStringMode = inputStream.instance.toStringMode();

    const identity = new Identity();
    identity._read(inputStream);
    s.push(`\nidentity = ${identityToString(identity, toStringMode)}`);

    const facet = StringSeqHelper.read(inputStream);
    s.push("\nfacet = ");
    if (facet.length > 0) {
        s.push(StringUtil.escapeString(facet[0], "", toStringMode));
    }

    const operation = inputStream.readString();
    s.push(`\noperation = ${operation}`);
}

function printRequest(s, inputStream) {
    const requestId = inputStream.readInt();
    s.push(`\nrequest id = ${requestId}`);
    if (requestId === 0) {
        s.push(" (oneway)");
    }

    printRequestHeader(s, inputStream);
}

function printBatchRequest(s, inputStream) {
    const batchRequestNum = inputStream.readInt();
    s.push(`\nnumber of requests = ${batchRequestNum}`);

    for (let i = 0; i < batchRequestNum; ++i) {
        s.push(`\nrequest #${i}:`);
        printRequestHeader(s, inputStream);
    }
}

function printReply(s, inputStream) {
    const requestId = inputStream.readInt();
    s.push(`\nrequest id = ${requestId}`);

    const replyStatus = ReplyStatus.valueOf(inputStream.readByte());
    s.push(`\nreply status = ${replyStatus.value} `);

    switch (replyStatus) {
        case ReplyStatus.Ok: {
            s.push("(ok)");
            break;
        }

        case ReplyStatus.UserException: {
            s.push("(user exception)");
            break;
        }

        case ReplyStatus.ObjectNotExist:
        case ReplyStatus.FacetNotExist:
        case ReplyStatus.OperationNotExist: {
            switch (replyStatus) {
                case ReplyStatus.ObjectNotExist: {
                    s.push("(object not exist)");
                    break;
                }

                case ReplyStatus.FacetNotExist: {
                    s.push("(facet not exist)");
                    break;
                }

                case ReplyStatus.OperationNotExist: {
                    s.push("(operation not exist)");
                    break;
                }

                default: {
                    DEV: console.assert(false);
                    break;
                }
            }

            printIdentityFacetOperation(s, inputStream);
            break;
        }

        case ReplyStatus.UnknownException:
        case ReplyStatus.UnknownLocalException:
        case ReplyStatus.UnknownUserException: {
            switch (replyStatus) {
                case ReplyStatus.UnknownException: {
                    s.push("(unknown exception)");
                    break;
                }

                case ReplyStatus.UnknownLocalException: {
                    s.push("(unknown local exception)");
                    break;
                }

                case ReplyStatus.UnknownUserException: {
                    s.push("(unknown user exception)");
                    break;
                }

                default: {
                    DEV: console.assert(false);
                    break;
                }
            }

            const unknown = inputStream.readString();
            s.push(`\nunknown = ${unknown}`);
            break;
        }

        default: {
            s.push("(unknown)");
            break;
        }
    }

    if (replyStatus === ReplyStatus.Ok || replyStatus === ReplyStatus.UserException) {
        const encodingVersion = inputStream.skipEncapsulation();
        if (!encodingVersion.equals(Encoding_1_0)) {
            s.push(`\nencoding = ${encodingVersionToString(encodingVersion)}`);
        }
    }
}

function printRequestHeader(s, inputStream) {
    printIdentityFacetOperation(s, inputStream);

    const mode = inputStream.readByte();
    s.push(`\nmode = ${mode} `);
    switch (OperationMode.valueOf(mode)) {
        case OperationMode.Normal: {
            s.push("(normal)");
            break;
        }

        case OperationMode.Nonmutating: {
            s.push("(nonmutating)");
            break;
        }

        case OperationMode.Idempotent: {
            s.push("(idempotent)");
            break;
        }

        default: {
            s.push("(unknown)");
            break;
        }
    }

    let sz = inputStream.readSize();
    s.push("\ncontext = ");
    while (sz-- > 0) {
        const key = inputStream.readString();
        const value = inputStream.readString();
        s.push(`${key}/${value}`);
        if (sz > 0) {
            s.push(", ");
        }
    }

    const ver = inputStream.skipEncapsulation();
    if (!ver.equals(Encoding_1_0)) {
        s.push(`\nencoding = ${encodingVersionToString(ver)}`);
    }
}

function printHeader(s, inputStream) {
    inputStream.readByte(); // Don't bother printing the magic number
    inputStream.readByte();
    inputStream.readByte();
    inputStream.readByte();

    //        const pMajor = inputStream.readByte();
    //        const pMinor = inputStream.readByte();
    //        s.push("\nprotocol version = " + pMajor + "." + pMinor);
    inputStream.readByte(); // major
    inputStream.readByte(); // minor

    //        const eMajor = inputStream.readByte();
    //        const eMinor = inputStream.readByte();
    //        s.push("\nencoding version = " + eMajor + "." + eMinor);
    inputStream.readByte(); // major
    inputStream.readByte(); // minor

    const type = inputStream.readByte();

    s.push(`\nmessage type = ${type} (${getMessageTypeAsString(type)})`);
    const compress = inputStream.readByte();
    s.push(`\ncompression status = ${compress} `);
    switch (compress) {
        case 0: {
            s.push("(not compressed; do not compress response, if any)");
            break;
        }

        case 1: {
            s.push("(not compressed; compress response, if any)");
            break;
        }

        case 2: {
            s.push("(compressed; compress response, if any)");
            break;
        }

        default: {
            s.push("(unknown)");
            break;
        }
    }

    const size = inputStream.readInt();
    s.push(`\nmessage size = ${size}`);
    return type;
}

function printMessage(s, inputStream, connection) {
    const type = printHeader(s, inputStream);

    switch (type) {
        case Protocol.closeConnectionMsg:
        case Protocol.validateConnectionMsg: {
            // We're done.
            break;
        }

        case Protocol.requestMsg: {
            printRequest(s, inputStream);
            break;
        }

        case Protocol.requestBatchMsg: {
            printBatchRequest(s, inputStream);
            break;
        }

        case Protocol.replyMsg: {
            printReply(s, inputStream);
            break;
        }

        default: {
            break;
        }
    }

    DEV: console.assert(connection !== null);

    s.push(`\ntransport = ${connection.type()}\n`);
    let connectionId = connection.endpoint().connectionId();
    if (connectionId.length > 0) {
        s.push(`connection ID = ${connectionId}\n`);
    }
    s.push(connection.toString());

    return type;
}

function getMessageTypeAsString(type) {
    switch (type) {
        case Protocol.requestMsg:
            return "request";
        case Protocol.requestBatchMsg:
            return "batch request";
        case Protocol.replyMsg:
            return "reply";
        case Protocol.closeConnectionMsg:
            return "close connection";
        case Protocol.validateConnectionMsg:
            return "validate connection";
        default:
            return "unknown";
    }
}

export class TraceUtil {
    static traceSend(outputStream, instance, connection, logger, traceLevels) {
        if (traceLevels.protocol >= 1) {
            const p = outputStream.pos;
            const inputStream = new InputStream(instance, outputStream.getEncoding(), outputStream.buffer);
            inputStream.pos = 0;

            const s = [];
            const type = printMessage(s, inputStream, connection);

            logger.trace(traceLevels.protocolCat, "sending " + getMessageTypeAsString(type) + " " + s.join(""));

            outputStream.pos = p;
        }
    }

    static traceRecv(inputStream, connection, logger, traceLevels) {
        if (traceLevels.protocol >= 1) {
            const p = inputStream.pos;
            inputStream.pos = 0;

            const s = [];
            const type = printMessage(s, inputStream, connection);

            logger.trace(traceLevels.protocolCat, "received " + getMessageTypeAsString(type) + " " + s.join(""));

            inputStream.pos = p;
        }
    }

    static trace(heading, inputStream, connection, logger, traceLevels) {
        if (traceLevels.protocol >= 1) {
            const p = inputStream.pos;
            inputStream.pos = 0;

            const s = [];
            s.push(heading);
            printMessage(s, inputStream, connection);

            logger.trace(traceLevels.protocolCat, s.join(""));
            inputStream.pos = p;
        }
    }
}
