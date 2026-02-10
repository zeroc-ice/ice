// Copyright (c) ZeroC, Inc.

import { ReferenceMode } from "./ReferenceMode.js";

export class FixedRequestHandler {
    constructor(ref, connection) {
        this._reference = ref;
        this._response = ref.getMode() == ReferenceMode.ModeTwoway;
        this._connection = connection;
    }

    sendAsyncRequest(out) {
        return out.invokeRemote(this._connection, this._response);
    }

    asyncRequestCanceled(out) {
        return this._connection.asyncRequestCanceled(out);
    }

    getConnection() {
        return this._connection;
    }
}
