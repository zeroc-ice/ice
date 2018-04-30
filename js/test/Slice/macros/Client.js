// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    const Ice = require("ice").Ice;
    const Test = require("Test").Test;

    function test(value)
    {
        if(!value)
        {
            throw new Error("test failed");
        }
    }

    function run(out)
    {
        try
        {
            out.write("testing Slice predefined macros... ");

            const d = new Test._Default();
            test(d.x == 10);
            test(d.y == 10);

            const nd = new Test.NoDefault();
            test(nd.x != 10);
            test(nd.y != 10);

            const c = new Test.JsOnly();
            test(c.lang == "js");
            test(c.version == Ice.intVersion());

            out.writeLine("ok");
            return Promise.resolve();
        }
        catch(ex)
        {
            return Promise.reject(ex);
        }
    }

    exports._test = run;
}(typeof global !== "undefined" && typeof global.process !== "undefined" ? module : undefined,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? require : this.Ice._require,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? exports : this));
