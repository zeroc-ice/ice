// Copyright (c) ZeroC, Inc.

import { Current } from "./Current.js";
import { FormatType } from "./FormatType.js";
import { OutgoingResponse } from "./OutgoingResponse.js";
import { LocalException } from "./LocalException.js";
import { UserException } from "./UserException.js";
import { DispatchException, RequestFailedException } from "./LocalExceptions.js";
import { OutputStream } from "./OutputStream.js";
import { Protocol } from "./Protocol.js";
import { Ice as Ice_BuiltinSequences } from "./BuiltinSequences.js";
const { StringSeqHelper } = Ice_BuiltinSequences;
import { Ice as Ice_ReplyStatus } from "./ReplyStatus.js";
const { ReplyStatus } = Ice_ReplyStatus;
import { Ice as Ice_Identity } from "./Identity.js";
const { Identity } = Ice_Identity;

Current.prototype.createOutgoingResponseWithResult = function (marshal, formatType = null) {
    const ostr = startReplyStream(this);
    if (this.requestId != 0) {
        try {
            ostr.startEncapsulation(this.encoding, formatType);
            marshal(ostr);
            ostr.endEncapsulation();
            return new OutgoingResponse(ostr);
        } catch (ex) {
            return this.createOutgoingResponseWithException(ex);
        }
    } else {
        DEV: console.assert("A one-way request cannot return a response");
        return new OutgoingResponse(ostr);
    }
};

Current.prototype.createEmptyOutgoingResponse = function () {
    const ostr = startReplyStream(this);
    if (this.requestId != 0) {
        try {
            ostr.writeEmptyEncapsulation(this.encoding);
        } catch (ex) {
            return this.createOutgoingResponse(ex);
        }
    }
    return new OutgoingResponse(ostr);
};

Current.prototype.createOutgoingResponseWithException = function (exception) {
    try {
        return createOutgoingResponseCore(this, exception);
    } catch {
        // Try a second time with the marshal exception. This should not fail.
        return createOutgoingResponseCore(this, exception);
    }
};

function startReplyStream(current, replyStatus = ReplyStatus.Ok) {
    if (current.requestId == 0) {
        return new OutputStream();
    } else {
        let ostr = new OutputStream(
            Protocol.currentProtocolEncoding,
            current.adapter.getCommunicator().instance.defaultsAndOverrides().defaultFormat,
        );
        ostr.writeBlob(Protocol.replyHdr);
        ostr.writeInt(current.requestId);
        ostr.writeByte(replyStatus.value);
        return ostr;
    }
}

function createOutgoingResponseCore(current, exception) {
    let ostr;

    if (current.requestId != 0) {
        // The default class format doesn't matter since we always encode user exceptions in Sliced format.
        ostr = new OutputStream(Protocol.currentProtocolEncoding);
        ostr.writeBlob(Protocol.replyHdr);
        ostr.writeInt(current.requestId);
    } else {
        ostr = new OutputStream();
    }

    let replyStatus;
    let exceptionId;
    let dispatchExceptionMessage = null;

    if (exception instanceof UserException) {
        exceptionId = null;
        replyStatus = ReplyStatus.UserException;

        if (current.requestId != 0) {
            ostr.writeByte(replyStatus.value);
            ostr.startEncapsulation(current.encoding, FormatType.SlicedFormat);
            ostr.writeException(exception);
            ostr.endEncapsulation();
        }
    } else if (exception instanceof DispatchException) {
        exceptionId = exception.ice_id();
        replyStatus = exception.replyStatus;
        dispatchExceptionMessage = exception.message;
    } else if (exception instanceof LocalException) {
        exceptionId = exception.ice_id();
        replyStatus = ReplyStatus.UnknownLocalException;
    } else {
        exceptionId = exception.name;
        replyStatus = ReplyStatus.UnknownException;
    }

    if (replyStatus.value > ReplyStatus.UserException.value && current.requestId != 0) {
        // Marshal a possibly unknown reply status value.
        ostr.writeByte(replyStatus.value);

        if (
            replyStatus === ReplyStatus.ObjectNotExist ||
            replyStatus === ReplyStatus.FacetNotExist ||
            replyStatus === ReplyStatus.OperationNotExist
        ) {
            let id = new Identity();
            let facet = "";
            let operation = "";
            if (exception instanceof RequestFailedException) {
                id = exception.id;
                facet = exception.facet;
                operation = exception.operation;
            }
            if (id.name.length == 0) {
                id = current.id;
                facet = current.facet;
            }
            if (operation.length == 0) {
                operation = current.operation;
            }

            id._write(ostr);

            if (facet.length == 0) {
                StringSeqHelper.write(ostr, []);
            } else {
                StringSeqHelper.write(ostr, [facet]);
            }
            ostr.writeString(operation);
        } else {
            // If the exception is a DispatchException, we keep its message as-is; otherwise, we create a custom
            // message. This message doesn't include the stack trace.
            dispatchExceptionMessage ??= `Dispatch failed with ${exceptionId}: ${exception.message}`;
            ostr.writeString(dispatchExceptionMessage);
        }
    }

    // The stack includes the class name and error message.
    return new OutgoingResponse(ostr, replyStatus, exceptionId, exceptionId != null ? exception.stack : null);
}
