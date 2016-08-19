// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

/* globals -Ice*/
var Ice = require("ice").Ice;

var write = function(msg)
{
    process.stdout.write(msg);
};

var writeLine = function(msg)
{
    this.write(msg + "\n");
};

var run = function(m)
{
    var id = new Ice.InitializationData();
    id.properties = Ice.createProperties(process.argv);
    var test = m.require("./Client").__test__;
    test({write: write, writeLine: writeLine}, id).catch(
        function(values)
        {
            let [ex, r] = values;
            console.log(ex.toString());
            if(r instanceof Ice.AsyncResult)
            {
                console.log("\nexception occurred in call to " + r.operation);
            }
            if(ex.stack)
            {
                console.log(ex.stack);
            }
            process.exit(1);
        });
};

exports.run = run;
