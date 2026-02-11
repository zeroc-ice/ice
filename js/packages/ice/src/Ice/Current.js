// Copyright (c) ZeroC, Inc.

export class Current {
    constructor(adapter, con, id, facet, operation, mode, ctx, requestId, encoding) {
        this.adapter = adapter;
        this.con = con;
        this.id = id;
        this.facet = facet;
        this.operation = operation;
        this.mode = mode;
        this.ctx = ctx;
        this.requestId = requestId;
        this.encoding = encoding;
    }
}
