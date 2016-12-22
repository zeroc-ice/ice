// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

/* global
    self : false,
    runTest : false
*/
var process = { argv : [] };

let out =
{
    write: function(msg)
    {
        self.postMessage({type:"write", message:msg});
    },
    writeLine: function(msg)
    {
        self.postMessage({type:"writeLine", message:msg});
    }
};

self.onmessage = function(e)
{
    try
    {
        for(let script in e.data.scripts)
        {
            self.importScripts(e.data.scripts[script]);
        }

        class Logger extends Ice.Logger
        {
            constructor(out)
            {
                super()
                this._out = out
            }

            write(message, indent)
            {
                if(indent)
                {
                    message = message.replace(/\n/g, "\n   ");
                }
                out.writeLine(message);
            }
        }

        let promise
        let initData = new Ice.InitializationData();
        initData.logger = new Logger(out);
        initData.properties = Ice.createProperties(e.data.args);
        process.argv = e.data.args;
        if(e.data.exe === "ClientBidir")
        {
            promise = _testBidir(out, initData);
        }
        else
        {
            promise = _test(out, initData);
        }
        promise.then(function() {
            self.postMessage({ type: "finished" });
        },
        function(ex) {
            self.postMessage({ type: "finished", exception:ex.toString() });
        });
    }
    catch(ex)
    {
        self.postMessage({ type: "finished", exception:ex.toString() });
    }
};
