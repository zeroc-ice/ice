// Copyright (c) ZeroC, Inc.

import { Buffer } from "./Buffer.js";

const _codeA = "A".charCodeAt(0);
const _codea = "a".charCodeAt(0);
const _code0 = "0".charCodeAt(0);

function encodeChar(uc) {
    if (uc < 26) {
        return String.fromCharCode(_codeA + uc);
    }

    if (uc < 52) {
        return String.fromCharCode(_codea + (uc - 26));
    }

    if (uc < 62) {
        return String.fromCharCode(_code0 + (uc - 52));
    }

    if (uc == 62) {
        return "+";
    }

    return "/";
}

function decodeChar(c) {
    if (c >= "A" && c <= "Z") {
        return c.charCodeAt(0) - _codeA;
    }

    if (c >= "a" && c <= "z") {
        return c.charCodeAt(0) - _codea + 26;
    }

    if (c >= "0" && c <= "9") {
        return c.charCodeAt(0) - _code0 + 52;
    }

    if (c == "+") {
        return 62;
    }

    return 63;
}


function isBase64(c) {
    return (c >= "A" && c <= "Z") || (c >= "a" && c <= "z") || (c >= "0" && c <= "9") || c == "+" || c == "/";
}

export class Base64 {
    static encode(buf) {
        if (buf === null || buf.length === 0) {
            return "";
        }

        const v = [];

        for (let i = 0; i < buf.length; i += 3) {
            const by1 = buf[i] & 0xff;
            const by2 = (i + 1 < buf.length) ? buf[i + 1] & 0xff : 0;
            const by3 = (i + 2 < buf.length) ? buf[i + 2] & 0xff : 0;

            v.push(encodeChar((by1 >> 2) & 0xff));
            v.push(encodeChar((((by1 & 0x3) << 4) | (by2 >> 4)) & 0xff));
            if (i + 1 < buf.length) {
                v.push(encodeChar((((by2 & 0xf) << 2) | (by3 >> 6)) & 0xff));
            } else {
                v.push("=");
            }
            if (i + 2 < buf.length) {
                v.push(encodeChar(by3 & 0x3f));
            } else {
                v.push("=");
            }
        }

        return v.join("");
    }

    static decode(str) {
        // First, remove any whitespace from the string.
        str = str.replace(/\s/g, "");

        // Reject any non-base64 characters.
        const paddingStart = str.search(/=*$/);
        for (let j = 0; j < paddingStart; j++) {
            const c = str[j];
            if (!isBase64(c)) {
                throw new Error(`invalid base64 character '${c}' (ordinal ${c.charCodeAt(0)})`);
            }
        }
        // Drop any padding characters at this point.
        str = str.slice(0, paddingStart);

        // Reject any base64 strings with only 1 out of 4 characters in the final sequence.
        // The final sequence may have 2, 3, or 4 characters, but 1 can't encode a full byte.
        if (str.length % 4 === 1) {
            throw new Error("invalid base64 string length");
        }

        const returnValue = new Buffer();
        returnValue.resize(Math.floor(str.length * 3 / 4));

        for (let i = 0; i < str.length; i += 4) {
            const c1 = str[i];
            const c2 = str[i + 1]; // Guaranteed okay because of the 'str.length % 4 === 1' check above.
            const c3 = (i + 2 < str.length) ? str[i + 2] : "=";
            const c4 = (i + 3 < str.length) ? str[i + 3] : "=";

            const by1 = decodeChar(c1) & 0xff;
            const by2 = decodeChar(c2) & 0xff;
            const by3 = decodeChar(c3) & 0xff;
            const by4 = decodeChar(c4) & 0xff;

            returnValue.put((by1 << 2) | (by2 >> 4));
            if (c3 !== "=") {
                returnValue.put(((by2 & 0xf) << 4) | (by3 >> 2));
            }
            if (c4 !== "=") {
                returnValue.put(((by3 & 0x3) << 6) | by4);
            }
        }

        return returnValue.getArrayAt(0);
    }
}
