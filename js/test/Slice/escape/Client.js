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
    var _await = require("Key")._await;

    var test = function(b)
    {
        if(!b)
        {
            throw new Error("test failed");
        }
    };

    var run = function(out, id)
    {
        return Ice.Promise.try(() =>
            {
                let communicator = Ice.initialize(id);
                out.write("testing enums... ");

                test(_await._var.base !== 0);
                out.writeLine("ok");

                out.write("testing structs... ");
                let s = new _await._break(10);
                test(s._while == 10);
                out.writeLine("ok");

                out.write("testing proxies... ");

                let p = _await.casePrx.uncheckedCast(
                    communicator.stringToProxy("hello:tcp -h 127.0.0.1 -p 12010").ice_timeout(100));
                test(p);

                return p._catch(10).then(() =>
                    {
                        test(false);
                    },
                    ex =>
                    {
                        test(ex instanceof Ice.LocalException);

                        let p = _await.typeofPrx.uncheckedCast(
                            communicator.stringToProxy("hello:tcp -h 127.0.0.1 -p 12010").ice_timeout(100));
                        test(p);
                        return p._default();
                    }
                ).then(() =>
                    {
                        test(false);
                    },
                    ex =>
                    {
                        test(ex instanceof Ice.LocalException);

                        out.writeLine("ok");

                        out.write("testing classes... ");

                        let d = new _await._delete(10, null, 10);
                        test(d._if === 10);
                        test(d._else === null);
                        test(d._export === 10);

                        let p = new _await._package(
                            new _await._break(100),
                            _await._var.base,
                            new Ice.Value(),
                            new Map(),
                            "",
                            _await.explicitPrx.uncheckedCast(communicator.stringToProxy("hello")));

                        test(p._for._while === 100);
                        test(p.goto === _await._var.base);
                        test(p._if instanceof Ice.Value);
                        test(p.internal instanceof Map);
                        test(p._debugger === "");
                        test(p._null instanceof _await.explicitPrx);
                    });
            }
        ).then(() => out.writeLine("ok"));
    };
    exports._test = run;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
