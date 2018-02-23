// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

/* global
    self : false,
    runTest : false,
    _server : false,
    _serveramd : false,
    _test : false
*/

class Output
{
    static write(message)
    {
        self.postMessage({type:"write", message:message});
    }

    static writeLine(message)
    {
        self.postMessage({type:"writeLine", message:message});
    }
}

self.onmessage = async (e) =>
    {
        try
        {
            for(let script of e.data.scripts)
            {
                self.importScripts(script);
            }

            class Logger extends Ice.Logger
            {
                write(message, indent)
                {
                    if(indent)
                    {
                        message = message.replace(/\n/g, "\n   ");
                    }
                    Output.writeLine(message);
                }
            }

            let promise;
            let initData = new Ice.InitializationData();
            initData.properties = Ice.createProperties(e.data.args);
            initData.logger = new Logger();
            if(e.data.exe === "Server" || e.data.exe === "ServerAMD")
            {
                let ready = new Ice.Promise();
                let test = e.data.exe === "Server" ? _server : _serveramd;
                promise = test(Output, initData, ready, e.data.args);
                await ready;
                self.postMessage({type:"ready"});
            }
            else
            {
                promise = _test(Output, initData, e.data.args);
            }

            await promise;
            self.postMessage({ type: "finished" });
        }
        catch(ex)
        {
            self.postMessage({ type: "finished", exception:ex.toString() });
        }
    };
