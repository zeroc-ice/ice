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
    const Ice = require("ice").Ice;
    const Test = require("Test").Test;

    async function allTests(out, communicator)
    {
        function test(value)
        {
            if(!value)
            {
                throw new Error("test failed");
            }
        }

        out.write("testing stringToProxy... ");
        let ref = "initial:default -p 12010";
        let base = communicator.stringToProxy(ref);
        test(base !== null);
        out.writeLine("ok");

        out.write("testing checked cast... ");
        let initial = await Test.InitialPrx.checkedCast(base);
        test(initial !== null);
        test(initial.equals(base));
        out.writeLine("ok");

        out.write("getting proxies for class hierarchy... ");

        let ca = await initial.caop();
        let cb = await initial.cbop();
        let cc = await initial.ccop();
        let cd = await initial.cdop();
        test(ca != cb);
        test(ca != cc);
        test(ca != cd);
        test(cb != cc);
        test(cb != cd);
        test(cc != cd);
        out.writeLine("ok");

        out.write("getting proxies for interface hierarchy... ");
        let ia = await initial.iaop();
        let ib1 = await initial.ib1op();
        let ib2 = await initial.ib2op();
        let ic = await initial.icop();
        test(ia != ib1);
        test(ia != ib2);
        test(ia != ic);
        test(ib1 != ic);
        test(ib2 != ic);
        out.writeLine("ok");

        out.write("invoking proxy operations on class hierarchy... ");
        let cao;
        let cbo;
        let cco;

        cao = await ca.caop(ca);
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
        cbo = await cb.cbop(cb);
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
        cco = await cc.ccop(cc);
        test(cco.equals(cc));
        out.writeLine("ok");

        out.write("ditto, but for interface hierarchy... ");
        let iao;
        let ib1o;
        let ib2o;
        let ico;

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
        ico = await ic.icop(ic);
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
    };

    async function run(out, initData)
    {
        let communicator;
        try
        {
            communicator = Ice.initialize(initData);
            await allTests(out, communicator);
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
    exports._runServer = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
