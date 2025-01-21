// Copyright (c) ZeroC, Inc.

import { Object } from "./Object.js";
import { ObjectPrx } from "./ObjectPrx.js";
import { Ice } from "./BuiltinSequences.js";
import { defineOperations } from "./Operation.js";

//
// Define the "built-in" operations for all Ice objects.
//
defineOperations(Object, ObjectPrx, ["::Ice::Object"], "::Ice::Object", {
    ice_ping: [undefined, 2, undefined, undefined, undefined, undefined, undefined],
    ice_isA: [undefined, 2, undefined, [1], [[7]], undefined, undefined],
    ice_id: [undefined, 2, undefined, [7], undefined, undefined, undefined],
    ice_ids: [undefined, 2, undefined, [Ice.StringSeqHelper], undefined, undefined, undefined],
});
