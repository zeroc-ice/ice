// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    var Ice = require("ice").Ice;
    var Test = require("Test").Test;

    var test = function(b)
    {
        if(!b)
        {
            throw new Error("test failed");
        }
    };

    var run = function(out)
    {
        return Ice.Promise.try(() =>
            {
                out.write("testing Slice predefined macros... ");

                let d = new Test._Default();
                test(d.x == 10);
                test(d.y == 10);

                let nd = new Test.NoDefault();
                test(nd.x != 10);
                test(nd.y != 10);

                let c = new Test.JsOnly();
                test(c.lang == "js");
                test(c.version == Ice.intVersion());
            }
        ).then(() => out.writeLine("ok"));
    };
    exports._test = run;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
