// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    var Ice = require("ice").Ice;
    var Test = require("Test").Test;

    var Promise = Ice.Promise;

    var allTests = function(out, communicator)
    {
        var ref, base, initial, ca, cb, cc, cd, ia, ib1, ib2, ic;
        var p = new Ice.Promise();
        var test = function(b)
        {
            if(!b)
            {
                try
                {
                    throw new Error("test failed");
                }
                catch(err)
                {
                    p.reject(err);
                    throw err;
                }
            }
        };

        Promise.try(
            function()
            {
                out.write("testing stringToProxy... ");
                ref = "initial:default -p 12010";
                base = communicator.stringToProxy(ref);
                test(base !== null);
                out.writeLine("ok");
                out.write("testing checked cast... ");
                return Test.InitialPrx.checkedCast(base);
            }
        ).then(
            function(obj)
            {
                initial = obj;
                test(initial !== null);
                test(initial.equals(base));
                out.writeLine("ok");

                out.write("getting proxies for class hierarchy... ");
                return Promise.all([initial.caop(), initial.cbop(),initial.ccop(), initial.cdop()]);
            }
        ).then(
            function(r)
            {
                var r1 = r[0], 
                    r2 = r[1],
                    r3 = r[2], 
                    r4 = r[3];
                ca = r1;
                cb = r2;
                cc = r3;
                cd = r4;

                test(ca !== cb);
                test(ca !== cc);
                test(ca !== cd);
                test(cb !== cc);
                test(cb !== cd);
                test(cc !== cd);
                out.writeLine("ok");
                out.write("getting proxies for interface hierarchy... ");

                return Promise.all([initial.iaop(), initial.ib1op(), initial.ib2op(), initial.icop()]);
            }
        ).then(
            function(r)
            {
                var [r1, r2, r3, r4] = r;
                ia = r1;
                ib1 = r2;
                test(ib1.ice_instanceof(Test.MB.IB1Prx));
                ib2 = r3;
                ic = r4;

                test(ia !== ib1);
                test(ia !== ib2);
                test(ia !== ic);
                test(ib1 !== ic);
                test(ib2 !== ic);
                out.writeLine("ok");
                out.write("invoking proxy operations on class hierarchy... ");
                return Promise.all([
                    ca.caop(ca),  // r1
                    ca.caop(cb),  // r2
                    ca.caop(cc),  // r3
                    cb.caop(ca),  // r4
                    cb.caop(cb),  // r5
                    cb.caop(cc),  // r6
                    cc.caop(ca),  // r7
                    cc.caop(cb),  // r8
                    cc.caop(cc),  // r9
                    cb.cbop(cb),  // r10
                    cb.cbop(cb),  // r11
                    cb.cbop(cc),  // r12
                    cb.cbop(cc),  // r13
                    cc.cbop(cb),  // r14
                    cc.cbop(cb),  // r15
                    cc.cbop(cc),  // r16
                    cc.cbop(cc),  // r17
                    cc.ccop(cc),  // r18
                    cc.ccop(cc),  // r19
                    cc.ccop(cc)]); // r20
            }
        ).then(
            function(r)
            {
                test(r[0].equals(ca));
                test(r[1].equals(cb));
                test(r[2].equals(cc));
                test(r[3].equals(ca));
                test(r[4].equals(cb));
                test(r[5].equals(cc));
                test(r[6].equals(ca));
                test(r[7].equals(cb));
                test(r[8].equals(cc));
                test(r[9].equals(cb));
                test(r[10].equals(cb));
                test(r[11].equals(cc));
                test(r[12].equals(cc));
                test(r[13].equals(cb));
                test(r[14].equals(cb));
                test(r[15].equals(cc));
                test(r[16].equals(cc));
                test(r[17].equals(cc));
                test(r[18].equals(cc));
                test(r[19].equals(cc));

                out.writeLine("ok");
                out.write("ditto, but for interface hierarchy... ");

                return Promise.all([
                    ia.iaop(ia),    // r1
                    ia.iaop(ib1),   // r2
                    ia.iaop(ib2),   // r3
                    ia.iaop(ic),    // r4
                    ib1.ib1op(ia),  // r5
                    ib1.iaop(ib1),  // r6
                    ib1.iaop(ib2),  // r7
                    ib1.iaop(ic),   // r8
                    ib2.iaop(ia),   // r9
                    ib2.iaop(ib1),  // r10
                    ib2.iaop(ib2),  // r11
                    ib2.iaop(ic),   // r12
                    ic.iaop(ia),    // r13
                    ic.iaop(ib1),   // r14
                    ic.iaop(ib2),   // r15
                    ic.iaop(ic),    // r16
                    ib1.ib1op(ib1), // r17
                    ib1.ib1op(ib1), // r18
                    ib1.ib1op(ic),  // r19
                    ib1.ib1op(ic),  // r20
                    ic.ib1op(ib1),  // r21
                    ic.ib1op(ib1),  // r22
                    ic.ib1op(ic),   // r23
                    ic.ib1op(ic),   // r24
                    ib2.ib2op(ib2), // r25
                    ib2.ib2op(ib2), // r26
                    ib2.ib2op(ic),  // r27
                    ib2.ib2op(ic),  // r28
                    ic.ib2op(ib2),  // r29
                    ic.ib2op(ib2),  // r30
                    ic.ib2op(ic),   // r31
                    ic.ib2op(ic),   // r32
                    ic.icop(ic),    // r33
                    ic.icop(ic),    // r34
                    ic.icop(ic),    // r35
                    ic.icop(ic)]);  // r36
            }
        ).then(
            function(r)
            {
                test(r[0].equals(ia));
                test(r[1].equals(ib1));
                test(r[2].equals(ib2));
                test(r[3].equals(ic));
                test(r[4].equals(ia));
                test(r[5].equals(ib1));
                test(r[6].equals(ib2));
                test(r[7].equals(ic));
                test(r[8].equals(ia));
                test(r[9].equals(ib1));
                test(r[10].equals(ib2));
                test(r[11].equals(ic));
                test(r[12].equals(ia));
                test(r[13].equals(ib1));
                test(r[14].equals(ib2));
                test(r[15].equals(ic));
                test(r[16].equals(ib1));
                test(r[17].equals(ib1));
                test(r[18].equals(ic));
                test(r[19].equals(ic));
                test(r[20].equals(ib1));
                test(r[21].equals(ib1));
                test(r[22].equals(ic));
                test(r[23].equals(ic));
                test(r[24].equals(ib2));
                test(r[25].equals(ib2));
                test(r[26].equals(ic));
                test(r[27].equals(ic));
                test(r[28].equals(ib2));
                test(r[29].equals(ib2));
                test(r[30].equals(ic));
                test(r[31].equals(ic));
                test(r[32].equals(ic));
                test(r[33].equals(ic));
                test(r[34].equals(ic));
                test(r[35].equals(ic));

                out.writeLine("ok");
                out.write("ditto, but for class implementing interfaces... ");

                return Promise.all([
                    cd.caop(cd),    // r1
                    cd.cbop(cd),    // r2
                    cd.ccop(cd),    // r3
                    cd.iaop(cd),    // r4
                    cd.ib1op(cd),   // r5
                    cd.ib2op(cd),   // r6
                    cd.cdop(cd),    // r7
                    cd.cdop(cd),    // r8
                    cd.cdop(cd),    // r9
                    cd.cdop(cd),    // r10
                    cd.cdop(cd),    // r11
                    cd.cdop(cd)]);  // r12
            }
        ).then(
            function(r)
            {
                test(r[0].equals(cd));
                test(r[1].equals(cd));
                test(r[2].equals(cd));
                test(r[3].equals(cd));
                test(r[4].equals(cd));
                test(r[5].equals(cd));
                test(r[6].equals(cd));
                test(r[7].equals(cd));
                test(r[8].equals(cd));
                test(r[9].equals(cd));
                test(r[10].equals(cd));
                test(r[11].equals(cd));
                out.writeLine("ok");
                return initial.shutdown();
            }
        ).then(p.resolve, p.reject);
        return p;
    };

    var run = function(out, id)
    {
        var c = Ice.initialize(id);
        return Promise.try(
            function()
            {
                return allTests(out, c);
            }
        ).finally(
            function()
            {
                return c.destroy();
            }
        );
    };
    exports._test = run;
    exports._clientAllTests = allTests;
    exports._runServer = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
