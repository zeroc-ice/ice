// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

let clientOut =
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

let serverOut =
{
    write: function(msg)
    {
        self.postMessage({type:"serverWrite", message:msg});
    },
    writeLine: function(msg)
    {
        self.postMessage({type:"serverWriteLine", message:msg});
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
        initData.properties = Ice.createProperties(e.data.args);
        process.argv = e.data.args;
        if(e.data.exe === "Server" || e.data.exe === "ServerAMD")
        {
            initData.logger = new Logger(serverOut);
            let ready = new Ice.Promise();
            let test = e.data.exe === "Server" ? _server : _serveramd;
            promise = test(serverOut, initData, ready);
            ready.then(() => self.postMessage({type:"ready"}));
        }
        else
        {
            initData.logger = new Logger(clientOut);
            promise = _test(clientOut, initData);
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
