//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/Buffer").Ice;

const _codeA = "A".charCodeAt(0);
const _codea = "a".charCodeAt(0);
const _code0 = "0".charCodeAt(0);

function encodeChar(uc)
{
    if(uc < 26)
    {
        return String.fromCharCode(_codeA + uc);
    }

    if(uc < 52)
    {
        return String.fromCharCode(_codea + (uc - 26));
    }

    if(uc < 62)
    {
        return String.fromCharCode(_code0 + (uc - 52));
    }

    if(uc == 62)
    {
        return "+";
    }

    return "/";
}

function decodeChar(c)
{
    if(c >= 'A' && c <= 'Z')
    {
        return c.charCodeAt(0) - _codeA;
    }

    if(c >= 'a' && c <= 'z')
    {
        return c.charCodeAt(0) - _codea + 26;
    }

    if(c >= '0' && c <= '9')
    {
        return c.charCodeAt(0) - _code0 + 52;
    }

    if(c == '+')
    {
        return 62;
    }

    return 63;
}

class Base64
{
    // Expects native Buffer
    static encode(buf)
    {
        if(buf === null || buf.length === 0)
        {
            return "";
        }

        const v = [];

        let by1;
        let by2;
        let by3;
        let by4;
        let by5;
        let by6;
        let by7;

        for(let i = 0; i < buf.length; i += 3)
        {
            by1 = buf[i] & 0xff;
            by2 = 0;
            by3 = 0;

            if((i + 1) < buf.length)
            {
                by2 = buf[i + 1] & 0xff;
            }

            if((i + 2) < buf.length)
            {
                by3 = buf[i + 2] & 0xff;
            }

            by4 = (by1 >> 2) & 0xff;
            by5 = (((by1 & 0x3) << 4) | (by2 >> 4)) & 0xff;
            by6 = (((by2 & 0xf) << 2) | (by3 >> 6)) & 0xff;
            by7 = by3 & 0x3f;

            v.push(encodeChar(by4));
            v.push(encodeChar(by5));

            if((i + 1) < buf.length)
            {
                v.push(encodeChar(by6));
            }
            else
            {
                v.push("=");
            }

            if((i + 2) < buf.length)
            {
                v.push(encodeChar(by7));
            }
            else
            {
                v.push("=");
            }
        }

        const retval = v.join("");
        const outString = [];
        let iter = 0;

        while((retval.length - iter) > 76)
        {
            outString.push(retval.substring(iter, iter + 76));
            outString.push("\r\n");
            iter += 76;
        }

        outString.push(retval.substring(iter));

        return outString.join("");
    }

    static decode(str) // Returns native Buffer
    {
        const newStr = [];

        for(let j = 0; j < str.length; j++)
        {
            const c = str.charAt(j);
            if(Base64.isBase64(c))
            {
                newStr.push(c);
            }
        }

        if(newStr.length === 0)
        {
            return null;
        }

        // Note: This is how we were previously computing the size of the return
        //       sequence.  The method below is more efficient (and correct).
        // size_t lines = str.size() / 78;
        // size_t totalBytes = (lines * 76) + (((str.size() - (lines * 78)) * 3) / 4);

        // Figure out how long the final sequence is going to be.
        const totalBytes = (newStr.length * 3 / 4) + 1;

        const retval = new Ice.Buffer();
        retval.resize(totalBytes);

        let by1;
        let by2;
        let by3;
        let by4;

        let c1;
        let c2;
        let c3;
        let c4;

        for(let i = 0; i < newStr.length; i += 4)
        {
            c1 = "A";
            c2 = "A";
            c3 = "A";
            c4 = "A";

            c1 = newStr[i];

            if((i + 1) < newStr.length)
            {
                c2 = newStr[i + 1];
            }

            if((i + 2) < newStr.length)
            {
                c3 = newStr[i + 2];
            }

            if((i + 3) < newStr.length)
            {
                c4 = newStr[i + 3];
            }

            by1 = decodeChar(c1) & 0xff;
            by2 = decodeChar(c2) & 0xff;
            by3 = decodeChar(c3) & 0xff;
            by4 = decodeChar(c4) & 0xff;

            retval.put((by1 << 2) | (by2 >> 4));

            if(c3 != "=")
            {
                retval.put(((by2 & 0xf) << 4) | (by3 >> 2));
            }

            if(c4 != "=")
            {
                retval.put(((by3 & 0x3) << 6) | by4);
            }
        }

        return retval.remaining > 0 ? retval.getArrayAt(0, retval.position) : retval.getArrayAt(0);
    }

    static isBase64(c)
    {
        if(c >= 'A' && c <= 'Z')
        {
            return true;
        }

        if(c >= 'a' && c <= 'z')
        {
            return true;
        }

        if(c >= '0' && c <= '9')
        {
            return true;
        }

        if(c == '+')
        {
            return true;
        }

        if(c == '/')
        {
            return true;
        }

        if(c == '=')
        {
            return true;
        }

        return false;
    }
}

Ice.Base64 = Base64;
module.exports.Ice = Ice;
