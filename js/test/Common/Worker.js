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
 
var Output =
{
    write: function(msg)
    {
        self.postMessage({type:"Write", message:msg});
    },
    writeLine: function(msg)
    {
        self.postMessage({type:"WriteLine", message:msg});
    }
};

self.onmessage = function(e)
{
    if(e.data.type == "RunTest")
    {
        var test = e.data.test;
        if(test.es5)
        {
            self.importScripts("/node_modules/babel-polyfill/dist/polyfill.js");
            self.importScripts("/node_modules/regenerator-runtime/runtime.js");
            self.importScripts("/lib/es5/Ice.js");
            self.importScripts("/test/Common/es5/Controller.js");
        }
        else
        {
            self.importScripts("/lib/Ice.js");
            self.importScripts("/test/Common/Controller.js");
        }
        self.importScripts("/test/Common/TestRunner.js");
        for(var i = 0; i < test.files.length; ++i)
        {
            var f = "/test/" + test.name + "/" + test.files[i];
            if(test.es5)
            {
                f = f.replace("/test/Ice/", "/test/Ice/es5/");
            }
            self.importScripts(f);
        }
        runTest(test.name, test.language, test.defaultHost, test.protocol, test.configurations, Output).then(
            function(r)
            {
                self.postMessage({type:"TestFinished", success:r});
            });
    }
};
