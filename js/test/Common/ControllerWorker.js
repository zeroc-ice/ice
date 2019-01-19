//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

/* global self, ControllerHelper, Ice */

class Output
{
    static write(message)
    {
        self.postMessage({type: "write", message: message});
    }

    static writeLine(message)
    {
        self.postMessage({type: "writeLine", message: message});
    }
}

self.onmessage = async e =>
    {
        try
        {
            for(const script of e.data.scripts)
            {
                self.importScripts(script);
            }
            const helper = new ControllerHelper(e.data.exe, Output);
            const cls = Ice._require(e.data.exe)[e.data.exe];
            const test = new cls();
            test.setControllerHelper(helper);
            const promise = test.run(e.data.args);
            await helper.waitReady();
            self.postMessage({type: "ready"});
            await promise;
            self.postMessage({type: "finished"});
        }
        catch(ex)
        {
            self.postMessage({type: "finished", exception: ex.toString()});
        }
    };
