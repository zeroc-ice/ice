// Copyright (c) ZeroC, Inc.

import { ToStringMode } from "./ToStringMode.js";
import { StringUtil } from "./StringUtil.js";

/**
 * Converts an object identity to a string.
 *
 * @param ident The object identity to convert.
 *
 * @param toStringMode Specifies if and how non-printable ASCII characters are escaped in the result.
 *
 * @return The string representation of the object identity.
 **/
export function identityToString(ident, toStringMode = ToStringMode.Unicode) {
    if (ident.category === null || ident.category.length === 0) {
        return StringUtil.escapeString(ident.name, "/", toStringMode);
    } else {
        return (
            StringUtil.escapeString(ident.category, "/", toStringMode) +
            "/" +
            StringUtil.escapeString(ident.name, "/", toStringMode)
        );
    }
}
