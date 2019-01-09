// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

(function(module, require, exports)
{
    const Test = require("Test").Test;
    const TestHelper = require("TestHelper").TestHelper;
    const test = TestHelper.test;

    class Client extends TestHelper
    {
        async allTests()
        {
            const communicator = this.communicator();
            const out = this.getWriter();

            out.write("testing stringToProxy... ");
            const ref = "initial:" + this.getTestEndpoint();
            const base = communicator.stringToProxy(ref);
            test(base !== null);
            out.writeLine("ok");

            out.write("testing checked cast... ");
            const initial = await Test.InitialPrx.checkedCast(base);
            test(initial !== null);
            test(initial.equals(base));
            out.writeLine("ok");

            out.write("getting proxies for class hierarchy... ");
            const ca = await initial.caop();
            const cb = await initial.cbop();
            const cc = await initial.ccop();
            const cd = await initial.cdop();
            test(ca != cb);
            test(ca != cc);
            test(ca != cd);
            test(cb != cc);
            test(cb != cd);
            test(cc != cd);
            out.writeLine("ok");

            out.write("getting proxies for interface hierarchy... ");
            const ia = await initial.iaop();
            const ib1 = await initial.ib1op();
            const ib2 = await initial.ib2op();
            const ic = await initial.icop();
            test(ia != ib1);
            test(ia != ib2);
            test(ia != ic);
            test(ib1 != ic);
            test(ib2 != ic);
            out.writeLine("ok");

            out.write("invoking proxy operations on class hierarchy... ");

            let cao = await ca.caop(ca);
            test(cao.equals(ca));
            cao = await ca.caop(cb);
            test(cao.equals(cb));
            cao = await ca.caop(cc);
            test(cao.equals(cc));
            cao = await cb.caop(ca);
            test(cao.equals(ca));
            cao = await cb.caop(cb);
            test(cao.equals(cb));
            cao = await cb.caop(cc);
            test(cao.equals(cc));
            cao = await cc.caop(ca);
            test(cao.equals(ca));
            cao = await cc.caop(cb);
            test(cao.equals(cb));
            cao = await cc.caop(cc);
            test(cao.equals(cc));

            cao = await cb.cbop(cb);
            test(cao.equals(cb));
            let cbo = await cb.cbop(cb);
            test(cbo.equals(cb));
            cao = await cb.cbop(cc);
            test(cao.equals(cc));
            cbo = await cb.cbop(cc);
            test(cbo.equals(cc));
            cao = await cc.cbop(cb);
            test(cao.equals(cb));
            cbo = await cc.cbop(cb);
            test(cbo.equals(cb));
            cao = await cc.cbop(cc);
            test(cao.equals(cc));
            cbo = await cc.cbop(cc);
            test(cbo.equals(cc));

            cao = await cc.ccop(cc);
            test(cao.equals(cc));
            cbo = await cc.ccop(cc);
            test(cbo.equals(cc));
            let cco = await cc.ccop(cc);
            test(cco.equals(cc));
            out.writeLine("ok");

            out.write("ditto, but for interface hierarchy... ");
            let iao;
            let ib1o;
            let ib2o;

            iao = await ia.iaop(ia);
            test(iao.equals(ia));
            iao = await ia.iaop(ib1);
            test(iao.equals(ib1));
            iao = await ia.iaop(ib2);
            test(iao.equals(ib2));
            iao = await ia.iaop(ic);
            test(iao.equals(ic));
            iao = await ib1.iaop(ia);
            test(iao.equals(ia));
            iao = await ib1.iaop(ib1);
            test(iao.equals(ib1));
            iao = await ib1.iaop(ib2);
            test(iao.equals(ib2));
            iao = await ib1.iaop(ic);
            test(iao.equals(ic));
            iao = await ib2.iaop(ia);
            test(iao.equals(ia));
            iao = await ib2.iaop(ib1);
            test(iao.equals(ib1));
            iao = await ib2.iaop(ib2);
            test(iao.equals(ib2));
            iao = await ib2.iaop(ic);
            test(iao.equals(ic));
            iao = await ic.iaop(ia);
            test(iao.equals(ia));
            iao = await ic.iaop(ib1);
            test(iao.equals(ib1));
            iao = await ic.iaop(ib2);
            test(iao.equals(ib2));
            iao = await ic.iaop(ic);
            test(iao.equals(ic));

            iao = await ib1.ib1op(ib1);
            test(iao.equals(ib1));
            ib1o = await ib1.ib1op(ib1);
            test(ib1o.equals(ib1));
            iao = await ib1.ib1op(ic);
            test(iao.equals(ic));
            ib1o = await ib1.ib1op(ic);
            test(ib1o.equals(ic));
            iao = await ic.ib1op(ib1);
            test(iao.equals(ib1));
            ib1o = await ic.ib1op(ib1);
            test(ib1o.equals(ib1));
            iao = await ic.ib1op(ic);
            test(iao.equals(ic));
            ib1o = await ic.ib1op(ic);
            test(ib1o.equals(ic));

            iao = await ib2.ib2op(ib2);
            test(iao.equals(ib2));
            ib2o = await ib2.ib2op(ib2);
            test(ib2o.equals(ib2));
            iao = await ib2.ib2op(ic);
            test(iao.equals(ic));
            ib2o = await ib2.ib2op(ic);
            test(ib2o.equals(ic));
            iao = await ic.ib2op(ib2);
            test(iao.equals(ib2));
            ib2o = await ic.ib2op(ib2);
            test(ib2o.equals(ib2));
            iao = await ic.ib2op(ic);
            test(iao.equals(ic));
            ib2o = await ic.ib2op(ic);
            test(ib2o.equals(ic));

            iao = await ic.icop(ic);
            test(iao.equals(ic));
            ib1o = await ic.icop(ic);
            test(ib1o.equals(ic));
            ib2o = await ic.icop(ic);
            test(ib2o.equals(ic));
            const ico = await ic.icop(ic);
            test(ico.equals(ic));
            out.writeLine("ok");

            out.write("ditto, but for class implementing interfaces... ");
            cao = await cd.caop(cd);
            test(cao.equals(cd));
            cbo = await cd.cbop(cd);
            test(cbo.equals(cd));
            cco = await cd.ccop(cd);
            test(cco.equals(cd));

            iao = await cd.iaop(cd);
            test(iao.equals(cd));
            ib1o = await cd.ib1op(cd);
            test(ib1o.equals(cd));
            ib2o = await cd.ib2op(cd);
            test(ib2o.equals(cd));

            cao = await cd.cdop(cd);
            test(cao.equals(cd));
            cbo = await cd.cdop(cd);
            test(cbo.equals(cd));
            cco = await cd.cdop(cd);
            test(cco.equals(cd));

            iao = await cd.cdop(cd);
            test(iao.equals(cd));
            ib1o = await cd.cdop(cd);
            test(ib1o.equals(cd));
            ib2o = await cd.cdop(cd);
            test(ib2o.equals(cd));
            out.writeLine("ok");

            await initial.shutdown();
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
