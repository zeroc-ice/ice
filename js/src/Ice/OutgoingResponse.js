// Copyright (c) ZeroC, Inc.

import { Protocol } from "./Protocol.js";
import { Ice as Ice_ReplyStatus } from "./ReplyStatus.js";
const { ReplyStatus } = Ice_ReplyStatus;

export class OutgoingResponse {
    constructor(outputStream, replyStatus, exceptionId, exceptionDetails) {
        this._outputStream = outputStream;
        this._replyStatus = replyStatus || ReplyStatus.Ok;
        if (replyStatus !== ReplyStatus.Ok.value) {
            this._exceptionId = exceptionId;
            this._exceptionDetails = exceptionDetails;
        }
    }

    get replyStatus() {
        return this._replyStatus;
    }

    get outputStream() {
        return this._outputStream;
    }

    get exceptionId() {
        return this._exceptionId;
    }

    get exceptionDetails() {
        return this._exceptionDetails;
    }

    get size() {
        return this._outputStream.isEmpty ? 0 : this._outputStream.size - Protocol.headerSize - 4;
    }
}
