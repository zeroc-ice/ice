
// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/Logger").Ice;
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
