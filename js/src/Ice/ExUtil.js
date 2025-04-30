// Copyright (c) ZeroC, Inc.

import { MarshalException } from "./LocalExceptions.js";
import { UnknownSlicedValue } from "./UnknownSlicedValue.js";

export function throwUOE(expectedType, v) {
    if (v instanceof UnknownSlicedValue) {
        throw new MarshalException(`The Slice loader did not find a class for type ID '${v.ice_id()}'.`);
    }

    throw new MarshalException(
        `Failed to unmarshal class with type ID '${expectedType}': the Slice loader returned class with type ID '${v.ice_id()}'.`,
    );
}

export function throwMemoryLimitException(requested, maximum) {
    throw new MarshalException(
        `Cannot unmarshal Ice message: the message size of ${requested} bytes exceeds the maximum allowed of ${maximum} bytes (see Ice.MessageSizeMax).`,
    );
}
