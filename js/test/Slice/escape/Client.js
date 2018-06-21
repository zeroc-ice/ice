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
    const _await = require("Key")._await;

    function test(value)
    {
        if(!value)
        {
            throw new Error("test failed");
        }
    }

    async function run(out, initData)
    {
        let communicator;
        try
        {
            communicator = Ice.initialize(initData);
            out.write("testing enums... ");

            test(_await._var.base !== 0);
            out.writeLine("ok");

            out.write("testing structs... ");
            const s = new _await._break(10);
            test(s._while == 10);
            out.writeLine("ok");

            out.write("testing proxies... ");

            let p = _await.casePrx.uncheckedCast(
                communicator.stringToProxy("hello:tcp -h 127.0.0.1 -p 12010").ice_timeout(100));
            test(p);

            try
            {
                await p._catch(10);
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.LocalException);
            }

            p = _await.typeofPrx.uncheckedCast(
                communicator.stringToProxy("hello:tcp -h 127.0.0.1 -p 12010").ice_timeout(100));
            test(p);

            try
            {
                await p._default();
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.LocalException);
            }
            out.writeLine("ok");

            out.write("testing classes... ");

            const d = new _await._delete(10, null, 10);
            test(d._if === 10);
            test(d._else === null);
            test(d._export === 10);

            p = new _await._package(
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
            out.writeLine("ok");
        }
        finally
        {
            if(communicator)
            {
                await communicator.destroy();
            }
        }
    }

    exports._test = run;
}(typeof global !== "undefined" && typeof global.process !== "undefined" ? module : undefined,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? require : this.Ice._require,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? exports : this));
