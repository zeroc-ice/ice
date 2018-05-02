// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const es5 = process.argv[2] === "--es5";
if(es5)
{
    process.argv.splice(2, 1);
}

const Ice = require(es5 ? "ice/src/es5" : "ice").Ice;

const write = function(msg)
{
    process.stdout.write(msg);
};

const writeLine = function(msg)
{
    this.write(msg + "\n");
};

const exception = function(ex)
{
    console.log(ex.toString());
    if(ex.stack)
    {
        console.log(ex.stack);
    }
    /* eslint-disable no-process-exit */
    process.exit(1);
    /* eslint-enable no-process-exit */
};

const id = new Ice.InitializationData();
id.properties = Ice.createProperties(process.argv);
const exe = process.argv[2];
let test = module.require(exe);
if(exe === "Server" || exe === "ServerAMD")
{
    const ready = new Ice.Promise();
    test = (exe === "Server") ? test._server : test._serveramd;
    test({write: write, writeLine: writeLine}, id, ready, process.argv).catch(exception);
    ready.then(() => console.log("server ready"));
}
else
{
    test._test({write: write, writeLine: writeLine}, id, process.argv).catch(exception);
}
