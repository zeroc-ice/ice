// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    const Ice = require("ice").Ice;
    const TestHelper = require("TestHelper").TestHelper;
    const _await = require("Key")._await;
    const test = TestHelper.test;

    class Client extends TestHelper
    {
        async allTests()
        {
            const communicator = this.communicator();
            const out = this.getWriter();

            out.write("testing enums... ");

            test(_await._var.base !== 0);
            out.writeLine("ok");

            out.write("testing structs... ");
            const s = new _await._break(10);
            test(s._while == 10);
            out.writeLine("ok");

            out.write("testing proxies... ");

            let p = _await.casePrx.uncheckedCast(
                communicator.stringToProxy("hello:" + this.getTestEndpoint()).ice_timeout(100));
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
                communicator.stringToProxy("hello:" + this.getTestEndpoint()).ice_timeout(100));
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

        async run(args)
        {
            let communicator;
            try
            {
                [communicator] = this.initialize(args);
                await this.allTests();
            }
            finally
            {
                if(communicator)
                {
                    await communicator.destroy();
                }
            }
        }
    }
    exports.Client = Client;
}(typeof global !== "undefined" && typeof global.process !== "undefined" ? module : undefined,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? require :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self.Ice._require : window.Ice._require,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? exports :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self : window));
