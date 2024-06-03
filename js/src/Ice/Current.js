//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { defineStruct } from "./Struct";
import { OperationMode } from "./OperationMode";
import Identity from "./Identity";
import { EncodingVersion } from "./Version";

/**
 *  Information about the current method invocation for servers. Each operation on the server has a
 *  <code>Current</code> as its implicit final parameter. <code>Current</code> is mostly used for Ice services. Most
 *  applications ignore this parameter.
 **/
class Current
{
    constructor(
        adapter = null,
        con = null,
        id = new Identity(),
        facet = "",
        operation = "",
        mode = OperationMode.Normal,
        ctx = null,
        requestId = 0,
        encoding = new EncodingVersion())
    {
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
};

defineStruct(Current, false, true);

export default Current;
