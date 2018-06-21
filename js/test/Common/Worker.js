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
    runTest : false
*/

class Output
{
    static write(msg)
    {
        self.postMessage({type: "Write", message: msg});
    }

    static writeLine(msg)
    {
        self.postMessage({type: "WriteLine", message: msg});
    }
}

self.onmessage = e =>
{
    try
    {
        const test = e.data.test;
        if(test.es5)
        {
            self.importScripts("/lib/es5/Ice.js");
            self.importScripts("/test/es5/Common/Controller.js");
            test.files = test.files.map(file => file.replace("/lib/Glacier2.js", "/lib/es5/Glacier2.js"));
            self.importScripts("/test/es5/Common/TestRunner.js");
        }
        else
        {
            self.importScripts("/lib/Ice.js");
            self.importScripts("/test/Common/Controller.js");
            self.importScripts("/test/Common/TestRunner.js");
        }

        for(const file of test.files)
        {
            if(file.indexOf("/") === -1)
            {
                self.importScripts(test.es5 ? `/test/es5/${test.name}/${file}` : `/test/${test.name}/${file}`);
            }
            else
            {
                self.importScripts(file);
            }
        }

        runTest(test.name, test.language, test.defaultHost, test.protocol, test.testcases, Output).then(
            result =>
                {
                    self.postMessage({type: "TestFinished", success: result});
                },
            ex =>
                {
                    Output.writeLine(ex.toString());
                    self.postMessage({type: "TestFinished", success: false});
                });
    }
    catch(ex)
    {
        Output.writeLine(ex.toString());
        self.postMessage({type: "TestFinished", success: false});
    }
};
