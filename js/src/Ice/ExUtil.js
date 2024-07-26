//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { UnexpectedObjectException, MemoryLimitException } from "./LocalExceptions.js";

export function throwUOE(expectedType, v) {
    const type = v.ice_id();
    throw new UnexpectedObjectException(
        "expected element of type `" + expectedType + "' but received `" + type + "'",
        type,
        expectedType,
    );
}

export function throwMemoryLimitException(requested, maximum) {
    throw new MemoryLimitException(
        "requested " + requested + " bytes, maximum allowed is " + maximum + " bytes (see Ice.MessageSizeMax)",
    );
}
