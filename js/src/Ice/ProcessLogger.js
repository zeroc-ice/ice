
// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(global){
    require("Ice/Logger");
    var Ice = global.Ice || {};
    var Logger = Ice.Logger;
    
    var processLogger = null;
    
    var getProcessLogger = function()
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

    var setProcessLogger = function(logger)
    {
        processLogger = logger;
    };

    Ice.getProcessLogger = getProcessLogger;
    Ice.setProcessLogger = setProcessLogger;
    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
