// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/Debug").Ice;    
const Debug = Ice.Debug;

Ice.StringUtil = class
{
    //
    // Return the index of the first character in str to
    // appear in match, starting from start. Returns -1 if none is
    // found.
    //
    static findFirstOf(str, match, start)
    {
        start = start === undefined ? 0 : start;
        for(let i = start; i < str.length; i++)
        {
            const ch = str.charAt(i);
            if(match.indexOf(ch) != -1)
            {
                return i;
            }
        }
        return -1;
    }
    //
    // Return the index of the first character in str which does
    // not appear in match, starting from start. Returns -1 if none is
    // found.
    //
    static findFirstNotOf(str, match, start)
    {
        start = start === undefined ? 0 : start;
        for(let i = start; i < str.length; i++)
        {
            const ch = str.charAt(i);
            if(match.indexOf(ch) == -1)
            {
                return i;
            }
        }
        return -1;
    }
    //
    // Add escape sequences (such as "\n", or "\007") to make a string
    // readable in ASCII. Any characters that appear in special are
    // prefixed with a backlash in the returned string.
    //
    static escapeString(s, special)
    {
        special = special === undefined ? null : special;
        if(special !== null)
        {
            for(let i = 0; i < special.length; ++i)
            {
                if(special.charCodeAt(i) < 32 || special.charCodeAt(i) > 126)
                {
                    throw new Error("special characters must be in ASCII range 32-126");
                }
            }
        }

        let result = [];
        for(let i = 0; i < s.length; ++i)
        {
            const c = s.charCodeAt(i);
            if(c < 128)
            {
                encodeChar(c, result, special);
            }
            else if(c > 127 && c < 2048)
            {
                encodeChar((c >> 6) | 192, result, special);
                encodeChar((c & 63) | 128, result, special);
            }
            else
            {
                encodeChar((c >> 12) | 224, result, special);
                encodeChar(((c >> 6) & 63) | 128, result, special);
                encodeChar((c & 63) | 128, result, special);
            }
        }

        return result.join("");
    }
    //
    // Remove escape sequences added by escapeString. Throws Error
    // for an invalid input string.
    //
    static unescapeString(s, start, end)
    {
        start = start === undefined ? 0 : start;
        end = end === undefined ? s.length : end;

        Debug.assert(start >= 0 && start <= end && end <= s.length);

        const arr = [];
        decodeString(s, start, end, arr);

        return arr.join("");
    }
    //
    // Split string helper; returns null for unmatched quotes
    //
    static splitString(str, delim)
    {
        const v = [];
        let s = "";
        let pos = 0;
        let quoteChar = null;
        while(pos < str.length)
        {
            if(quoteChar === null && (str.charAt(pos) === '"' || str.charAt(pos) === '\''))
            {
                quoteChar = str.charAt(pos++);
                continue; // Skip the quote.
            }
            else if(quoteChar === null && str.charAt(pos) === '\\' && pos + 1 < str.length &&
                    (str.charAt(pos + 1) === '"' || str.charAt(pos + 1) === '\''))
            {
                ++pos; // Skip the backslash
            }
            else if(quoteChar !== null && str.charAt(pos) === '\\' && pos + 1 < str.length &&
                    str.charAt(pos + 1) === quoteChar)
            {
                ++pos; // Skip the backslash
            }
            else if(quoteChar !== null && str.charAt(pos) === quoteChar)
            {
                ++pos;
                quoteChar = null;
                continue; // Skip the quote.
            }
            else if(delim.indexOf(str.charAt(pos)) !== -1)
            {
                if(quoteChar === null)
                {
                    ++pos;
                    if(s.length > 0)
                    {
                        v.push(s);
                        s = "";
                    }
                    continue;
                }
            }

            if(pos < str.length)
            {
                s += str.charAt(pos++);
            }
        }

        if(s.length > 0)
        {
            v.push(s);
        }
        if(quoteChar !== null)
        {
            return null; // Unmatched quote.
        }

        return v;
    }
    //
    // If a single or double quotation mark is found at the start position,
    // then the position of the matching closing quote is returned. If no
    // quotation mark is found at the start position, then 0 is returned.
    // If no matching closing quote is found, then -1 is returned.
    //
    static checkQuote(s, start)
    {
        start = start === undefined ? 0 : start;

        let quoteChar = s.charAt(start);
        if(quoteChar == '"' || quoteChar == '\'')
        {
            start++;
            let pos;
            while(start < s.length && (pos = s.indexOf(quoteChar, start)) != -1)
            {
                if(s.charAt(pos - 1) != '\\')
                {
                    return pos;
                }
                start = pos + 1;
            }
            return -1; // Unmatched quote
        }
        return 0; // Not quoted
    }
    static hashCode(s)
    {
        let hash = 0;
        for(let i = 0; i < s.length; i++)
        {
            hash = 31 * hash + s.charCodeAt(i);
        }
        return hash;
    }
    static toInt(s)
    {
        const n = parseInt(s, 10);
        if(isNaN(n))
        {
            throw new Error("conversion of `" + s + "' to int failed");
        }
        return n;
    }
};
module.exports.Ice = Ice;

//
// Write the byte b as an escape sequence if it isn't a printable ASCII
// character and append the escape sequence to sb. Additional characters
// that should be escaped can be passed in special. If b is any of these
// characters, b is preceded by a backslash in sb.
//
function encodeChar(b, sb, special)
{
    switch(b)
    {
        case 92: // '\\'
        {
            sb.push("\\\\");
            break;
        }
        case 39: // '\''
        {
            sb.push("\\'");
            break;
        }
        case 34: // '"'
        {
            sb.push("\\\"");
            break;
        }
        case 8: // '\b'
        {
            sb.push("\\b");
            break;
        }
        case 12: // '\f'
        {
            sb.push("\\f");
            break;
        }
        case 10: // '\n'
        {
            sb.push("\\n");
            break;
        }
        case 13: // '\r'
        {
            sb.push("\\r");
            break;
        }
        case 9: // '\t'
        {
            sb.push("\\t");
            break;
        }
        default:
        {
            if(!(b >= 32 && b <= 126))
            {
                sb.push('\\');
                const octal = b.toString(8);
                //
                // Add leading zeroes so that we avoid problems during
                // decoding. For example, consider the encoded string
                // \0013 (i.e., a character with value 1 followed by
                // the character '3'). If the leading zeroes were omitted,
                // the result would be incorrectly interpreted by the
                // decoder as a single character with value 11.
                //
                for(let j = octal.length; j < 3; j++)
                {
                    sb.push('0');
                }
                sb.push(octal);
            }
            else
            {
                const c = String.fromCharCode(b);
                if(special !== null && special.indexOf(c) !== -1)
                {
                    sb.push('\\');
                    sb.push(c);
                }
                else
                {
                    sb.push(c);
                }
            }
        }
    }
}
function checkChar(s, pos)
{
    const n = s.charCodeAt(pos);
    if(!(n >= 32 && n <= 126))
    {
        let msg;
        if(pos > 0)
        {
            msg = "character after `" + s.substring(0, pos) + "'";
        }
        else
        {
            msg = "first character";
        }
        msg += " is not a printable ASCII character (ordinal " + n + ")";
        throw new Error(msg);
    }
    return n;
}

//
// Decode the character or escape sequence starting at start and return it.
// nextStart is set to the index of the first character following the decoded
// character or escape sequence.
//
function decodeChar(s, start, end, nextStart)
{
    Debug.assert(start >= 0);
    Debug.assert(end <= s.length);

    if(start >= end)
    {
        throw new Error("EOF while decoding string");
    }

    let c;

    if(s.charAt(start) != '\\')
    {
        c = checkChar(s, start++);
    }
    else
    {
        if(start + 1 == end)
        {
            throw new Error("trailing backslash");
        }
        switch(s.charAt(++start))
        {
            case '\\':
            case '\'':
            case '"':
            {
                c = s.charCodeAt(start++);
                break;
            }
            case 'b':
            {
                ++start;
                c = "\b".charCodeAt(0);
                break;
            }
            case 'f':
            {
                ++start;
                c = "\f".charCodeAt(0);
                break;
            }
            case 'n':
            {
                ++start;
                c = "\n".charCodeAt(0);
                break;
            }
            case 'r':
            {
                ++start;
                c = "\r".charCodeAt(0);
                break;
            }
            case 't':
            {
                ++start;
                c = "\t".charCodeAt(0);
                break;
            }
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            {
                const octalChars = "01234567";
                let val = 0;
                for(let j = 0; j < 3 && start < end; ++j)
                {
                    const ch = s.charAt(start++);
                    if(octalChars.indexOf(ch) == -1)
                    {
                        --start;
                        break;
                    }
                    val = val * 8 + parseInt(ch);
                }
                if(val > 255)
                {
                    throw new Error("octal value \\" + val.toString(8) + " (" + val + ") is out of range");
                }
                c = val;
                break;
            }
            default:
            {
                c = checkChar(s, start++);
                break;
            }
        }
    }
    nextStart.value = start;
    return c;
}

//
// Remove escape sequences from s and append the result to sb.
// Return true if successful, false otherwise.
//
function decodeString(s, start, end, arr)
{
    let nextStart = { 'value': 0 };
    while(start < end)
    {
        const c = decodeChar(s, start, end, nextStart);
        start = nextStart.value;

        if(c < 128)
        {
            arr.push(String.fromCharCode(c));
        }
        else if(c > 191 && c < 224)
        {
            const c2 = decodeChar(s, start, end, nextStart);
            start = nextStart.value;
            arr.push(String.fromCharCode(((c & 31) << 6) | (c2 & 63)));
        }
        else
        {
            const c2 = decodeChar(s, start, end, nextStart);
            start = nextStart.value;
            const c3 = decodeChar(s, start, end, nextStart);
            start = nextStart.value;
            arr.push(String.fromCharCode(((c & 15) << 12) | ((c2 & 63) << 6) | (c3 & 63)));
        }
    }
}
