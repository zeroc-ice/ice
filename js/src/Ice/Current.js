//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { Ice as Ice_OperationMode } from "./OperationMode.js";
const { OperationMode } = Ice_OperationMode;
import { Ice as Ice_Identity } from "./Identity.js";
const { Identity } = Ice_Identity;
import { Ice as Ice_Version } from "./Version.js";
const { EncodingVersion } = Ice_Version;
/**
 *  Information about the current method invocation for servers. Each operation on the server has a
 *  <code>Current</code> as its implicit final parameter. <code>Current</code> is mostly used for Ice services. Most
 *  applications ignore this parameter.
 **/
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
