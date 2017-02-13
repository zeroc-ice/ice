// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
var fs = require("fs");

function writeSync(stream, msg)
{
    var data = new Buffer(msg + "\n");
    fs.writeSync(stream.fd, data, 0, data.length, stream.pos);
}

Ice.Debug =
{
    assert: function(b, msg)
    {
        if(!b)
        {
            writeSync(process.stderr, msg === undefined ? "assertion failed" : msg);
            writeSync(process.stderr, new Error().stack);
            process.exit(1);
        }
    }
};
module.exports.Ice = Ice;
