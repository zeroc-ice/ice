// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

require("ice");

class ControllerHelper
{
    write(msg)
    {
        process.stdout.write(msg);
    }

    writeLine(msg)
    {
        process.stdout.write(msg + "\n");
    }

    serverReady()
    {
        console.log("server ready");
    }
}

(async function()
 {
     try
     {
         const name = process.argv[2];
         const cls = module.require(name)[name];
         const test = new cls();
         test.setControllerHelper(new ControllerHelper());
         await test.run(process.argv);
     }
     catch(ex)
     {
         console.log(ex);
         /* eslint-disable no-process-exit */
         process.exit(1);
         /* eslint-enable no-process-exit */
     }
 }());
