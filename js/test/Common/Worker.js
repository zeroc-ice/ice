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
    try
    {
        if(e.data.type == "RunTest")
        {
            var test = e.data.test;
            self.importScripts("/lib/Ice.js");
            self.importScripts("/test/Common/Controller.js");
            self.importScripts("/test/Common/TestRunner.js");
            for(var i = 0; i < test.files.length; ++i)
            {
                self.importScripts("/test/" + test.name + "/" + test.files[i]);
            }

            runTest(test.name, test.language, test.defaultHost, test.protocol, test.configurations, Output).then(
                function(r)
                {
                    self.postMessage({type:"TestFinished", success:r});
                }).exception(
                    function(ex)
                    {
                        Output.writeLine(ex.toString());
                        self.postMessage({type:"TestFinished", success:false});
                    });
        }
    }
    catch(ex)
    {
        Output.writeLine(ex.toString());
        self.postMessage({type:"TestFinished", success:false});
    }
};
