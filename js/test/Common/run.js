// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var es5 = process.argv[2] === "--es5";
if(es5)
{
    process.argv.splice(2, 1);
}

var Ice = require(es5 ? "ice/src/es5" : "ice").Ice;

var write = function(msg)
{
    process.stdout.write(msg);
};

var writeLine = function(msg)
{
    this.write(msg + "\n");
};

var exception = function(ex)
{
    console.log(ex.toString());
    if(ex.stack)
    {
        console.log(ex.stack);
    }
    process.exit(1);
};

var id = new Ice.InitializationData();
id.properties = Ice.createProperties(process.argv);
var exe = process.argv[2];
var test = module.require(exe);
if(exe === "Server" || exe === "ServerAMD")
{
    var ready = new Ice.Promise();
    test = exe === "Server" ? test._server : test._serveramd;
    test({write: write, writeLine: writeLine}, id, ready, process.argv).catch(exception);
    ready.then(() => console.log("server ready"));
}
else
{
    test._test({write: write, writeLine: writeLine}, id, process.argv).catch(exception);
}
