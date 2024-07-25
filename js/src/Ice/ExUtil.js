//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { MarshalException } from "./LocalExceptions.js";
import { UnknownSlicedValue } from "./UnknownSlicedValue.js";

export function throwUOE(expectedType, v) {
    // If the object is an unknown sliced object, we didn't find an value factory.
    if (v instanceof UnknownSlicedValue) {
        throw new MarshalException(`Cannot find value factory to unmarshal class with type ID '{v.ice_id()}'.`);
    }

    const type = v.ice_id();
    let expected = expectedType.ice_staticId(); // TODO add ice_staticId

    throw new MarshalException(
        `Failed to unmarshal class with type ID '${expected}': value factory returned class with type ID '${type}'.`,
    );
}

export function throwMemoryLimitException(requested, maximum) {
    throw new MarshalException(
        `Cannot unmarshal Ice message: the message size of ${requested} bytes exceeds the maximum allowed of ${maximum} bytes (see Ice.MessageSizeMax).`,
    );
}
