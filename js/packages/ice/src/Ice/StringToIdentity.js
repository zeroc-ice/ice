// Copyright (c) ZeroC, Inc.

import { ParseException } from "./LocalExceptions.js";
import { StringUtil } from "./StringUtil.js";
import { Ice as Ice_Identity } from "./Identity.js";
const { Identity } = Ice_Identity;

export function stringToIdentity(s) {
    const ident = new Identity();

    //
    // Find unescaped separator; note that the string may contain an escaped
    // backslash before the separator.
    //
    let slash = -1;
    let pos = 0;
    while ((pos = s.indexOf("/", pos)) !== -1) {
        let escapes = 0;
        while (pos - escapes > 0 && s.charAt(pos - escapes - 1) == "\\") {
            escapes++;
        }

        //
        // We ignore escaped escapes
        //
        if (escapes % 2 === 0) {
            if (slash == -1) {
                slash = pos;
            } else {
                //
                // Extra unescaped slash found.
                //
                throw new ParseException(`unescaped backslash in identity '${s}'`);
            }
        }
        pos++;
    }

    if (slash == -1) {
        ident.category = "";
        try {
            ident.name = StringUtil.unescapeString(s, 0, s.length, "/");
        } catch (e) {
            throw new ParseException(`invalid identity name '${s}': ${e.toString()}`);
        }
    } else {
        try {
            ident.category = StringUtil.unescapeString(s, 0, slash, "/");
        } catch (e) {
            throw new ParseException(`invalid category in identity '${s}': ${e.toString()}`);
        }
        if (slash + 1 < s.length) {
            try {
                ident.name = StringUtil.unescapeString(s, slash + 1, s.length, "/");
            } catch (e) {
                throw new ParseException(`invalid name in identity '${s}': ${e.toString()}`);
            }
        } else {
            ident.name = "";
        }
    }

    return ident;
}
