//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

require("../Ice/Debug");
require("../Ice/LocalException");

class RetryException extends Error
{
    constructor(ex)
    {
        super();
        if(ex instanceof Ice.LocalException)
        {
            this._ex = ex;
        }
        else
        {
            Ice.Debug.assert(ex instanceof RetryException);
            this._ex = ex._ex;
        }
    }

    get inner()
    {
        return this._ex;
    }
}

Ice.RetryException = RetryException;
module.exports.Ice = Ice;
