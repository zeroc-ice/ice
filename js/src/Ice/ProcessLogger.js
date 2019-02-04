
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/LoggerI").Ice;
const Logger = Ice.Logger;

let processLogger = null;

Ice.getProcessLogger = function()
{
    if(processLogger === null)
    {
        //
        // TODO: Would be nice to be able to use process name as prefix by default.
        //
        processLogger = new Logger("", "");
    }

    return processLogger;
};

Ice.setProcessLogger = function(logger)
{
    processLogger = logger;
};

module.exports.Ice = Ice;
