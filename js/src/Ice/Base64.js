// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/Buffer").Ice;

var Buffer = Ice.Buffer;

var Base64 = {};

var _codeA = "A".charCodeAt(0);
var _codea = "a".charCodeAt(0);
var _code0 = "0".charCodeAt(0);

Base64.encode = function(buf) // Expects native Buffer
{
    if(buf === null || buf.length === 0)
    {
        return "";
    }

    var base64Bytes = (((buf.length * 4) / 3) + 1);
    var newlineBytes = (((base64Bytes * 2) / 76) + 1);
    var totalBytes = base64Bytes + newlineBytes;

    var v = [];

    var by1;
    var by2;
    var by3;
    var by4;
    var by5;
    var by6;
    var by7;

    for(var i = 0; i < buf.length; i += 3)
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

    var retval = v.join("");
    var outString = [];
    var iter = 0;

    while((retval.length - iter) > 76)
    {
        outString.push(retval.substring(iter, iter + 76));
        outString.push("\r\n");
        iter += 76;
    }

    outString.push(retval.substring(iter));

    return outString.join("");
};

Base64.decode = function(str) // Returns native Buffer
{
    var newStr = [];

    for(var j = 0; j < str.length; j++)
    {
        var c = str.charAt(j);
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
    var totalBytes = (newStr.length * 3 / 4) + 1;

    var retval = new Buffer();
    retval.resize(totalBytes);

    var by1;
    var by2;
    var by3;
    var by4;

    var c1;
    var c2;
    var c3;
    var c4;

    var off = 0;

    for(var i = 0; i < newStr.length; i += 4)
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
};

Base64.isBase64 = function(c)
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
};

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

Ice.Base64 = Base64;
module.exports.Ice = Ice;
