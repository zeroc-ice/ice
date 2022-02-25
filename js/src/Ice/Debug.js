//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

/* eslint no-sync: "off" */
/* eslint no-process-exit: "off" */

const Ice = require("../Ice/ModuleRegistry").Ice;
require("../Ice/Exception");

let Debug = {};

if (typeof process !== 'undefined')
{
    const fs = require("fs");
    Debug = class
    {
        static assert(b, msg)
        {
            if(!b)
            {
                fs.writeSync(process.stderr.fd, msg === undefined ? "assertion failed" : msg);
                fs.writeSync(process.stderr.fd, new Error().stack);
                process.exit(1);
            }
        }
    }
}
else
{
    class AssertionFailedException extends Error
    {
        constructor(message)
        {
            super();
            Ice.Exception.captureStackTrace(this);
            this.message = message;
        }
    }
    Ice.AssertionFailedException = AssertionFailedException;

    Debug = class
    {
        static assert(b, msg)
        {
            if(!b)
            {
                console.log(msg === undefined ? "assertion failed" : msg);
                console.log(new Error().stack);
                throw new Ice.AssertionFailedException(msg === undefined ? "assertion failed" : msg);
            }
        }
    }
}

Ice.Debug = Debug;
module.exports.Ice = Ice;
