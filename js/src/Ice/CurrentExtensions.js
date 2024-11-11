//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { Current } from "./Current.js";
import { FormatType } from "./FormatType.js";
import { Debug } from "./Debug.js";
import { OutgoingResponse } from "./OutgoingResponse.js";
import { ReplyStatus } from "./ReplyStatus.js";
import { Exception } from "./Exception.js";
import { LocalException } from "./LocalException.js";
import { UserException } from "./UserException.js";
import {
    ObjectNotExistException,
    FacetNotExistException,
    OperationNotExistException,
    UnknownException,
    UnknownUserException,
    UnknownLocalException,
    RequestFailedException,
} from "./LocalExceptions.js";
import { OutputStream } from "./OutputStream.js";
import { Protocol } from "./Protocol.js";
import { Ice as Ice_BuiltinSequences } from "./BuiltinSequences.js";
const { StringSeqHelper } = Ice_BuiltinSequences;

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
    } catch (ex) {
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
        ostr = new OutputStream(Protocol.currentProtocolEncoding);
        ostr.writeBlob(Protocol.replyHdr);
        ostr.writeInt(current.requestId);
    } else {
        ostr = new OutputStream();
    }

    let replyStatus;
    let exceptionId;
    let exceptionDetails = null;
    let unknownExceptionMessage = null;

    if (exception instanceof RequestFailedException) {
        exceptionId = exception.ice_id();

        if (exception instanceof ObjectNotExistException) {
            replyStatus = ReplyStatus.ObjectNotExist;
        } else if (exception instanceof FacetNotExistException) {
            replyStatus = ReplyStatus.FacetNotExist;
        } else if (exception instanceof OperationNotExistException) {
            replyStatus = ReplyStatus.OperationNotExist;
        } else {
            throw new MarshalException("Unexpected exception type");
        }

        let id = exception.id;
        let facet = exception.facet;
        if (id.name.length == 0) {
            id = current.id;
            facet = current.facet;
        }
        const operation = exception.operation.length == 0 ? current.operation : exception.operation;

        exceptionDetails = RequestFailedException.createMessage(exception.name, id, facet, operation);

        if (current.requestId != 0) {
            ostr.writeByte(replyStatus.value);
            id._write(ostr);

            if (facet.length == 0) {
                StringSeqHelper.write(ostr, []);
            } else {
                StringSeqHelper.write(ostr, [facet]);
            }
            ostr.writeString(operation);
        }
    } else if (exception instanceof UserException) {
        exceptionId = exception.ice_id();
        replyStatus = ReplyStatus.UserException;

        if (current.requestId != 0) {
            ostr.writeByte(replyStatus.value);
            ostr.startEncapsulation(current.encoding, FormatType.SlicedFormat);
            ostr.writeException(exception);
            ostr.endEncapsulation();
        }
    } else if (exception instanceof UnknownLocalException) {
        exceptionId = exception.ice_id();
        replyStatus = ReplyStatus.UnknownLocalException;
        unknownExceptionMessage = exception.message;
    } else if (exception instanceof UnknownUserException) {
        exceptionId = exception.ice_id();
        replyStatus = ReplyStatus.UnknownUserException;
        unknownExceptionMessage = exception.message;
    } else if (exception instanceof UnknownException) {
        exceptionId = exception.ice_id();
        replyStatus = ReplyStatus.UnknownException;
        unknownExceptionMessage = exception.message;
    } else if (exception instanceof LocalException) {
        exceptionId = exception.ice_id();
        replyStatus = ReplyStatus.UnknownLocalException;
    } else if (exception instanceof Exception) {
        exceptionId = exception.ice_id();
        replyStatus = ReplyStatus.UnknownException;
    } else {
        replyStatus = ReplyStatus.UnknownException;
        exceptionId = exception.name;
    }

    if (
        current.requestId != 0 &&
        (replyStatus == ReplyStatus.UnknownUserException ||
            replyStatus == ReplyStatus.UnknownLocalException ||
            replyStatus == ReplyStatus.UnknownException)
    ) {
        ostr.writeByte(replyStatus.value);
        // If the exception is an UnknownXxxException, we keep its message as-is; otherwise, we create a custom
        // message. This message doesn't include the stack trace.
        unknownExceptionMessage =
            unknownExceptionMessage || `Dispatch failed with ${exceptionId}: ${exception.message}`;
        ostr.writeString(unknownExceptionMessage);
    }

    return new OutgoingResponse(ostr, replyStatus, exceptionId, exceptionDetails ?? `${exception}\n${exception.stack}`);
}
