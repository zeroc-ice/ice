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
                    p.fail(err);
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
                return Promise.all(initial.caop(),
                                   initial.cbop(),
                                   initial.ccop(),
                                   initial.cdop());
            }
        ).then(
            function(r1, r2, r3, r4)
            {
                ca = r1[0];
                cb = r2[0];
                cc = r3[0];
                cd = r4[0];

                test(ca !== cb);
                test(ca !== cc);
                test(ca !== cd);
                test(cb !== cc);
                test(cb !== cd);
                test(cc !== cd);
                out.writeLine("ok");
                out.write("getting proxies for interface hierarchy... ");

                return Promise.all(initial.iaop(),
                                   initial.ib1op(),
                                   initial.ib2op(),
                                   initial.icop());
            }
        ).then(
            function(r1, r2, r3, r4)
            {
                ia = r1[0];
                ib1 = r2[0];
                test(ib1.ice_instanceof(Test.MB.IB1Prx));
                ib2 = r3[0];
                ic = r4[0];

                test(ia !== ib1);
                test(ia !== ib2);
                test(ia !== ic);
                test(ib1 !== ic);
                test(ib2 !== ic);
                out.writeLine("ok");
                out.write("invoking proxy operations on class hierarchy... ");
                return Promise.all(
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
                    cc.ccop(cc)); // r20
            }
        ).then(
            function(r1, r2, r3, r4, r5, r6, r7, r8, r9, r10,
                     r11, r12, r13, r14, r15, r16, r17, r18, r19, r20)
            {
                test(r1[0].equals(ca));
                test(r2[0].equals(cb));
                test(r3[0].equals(cc));
                test(r4[0].equals(ca));
                test(r5[0].equals(cb));
                test(r6[0].equals(cc));
                test(r7[0].equals(ca));
                test(r8[0].equals(cb));
                test(r9[0].equals(cc));
                test(r10[0].equals(cb));
                test(r11[0].equals(cb));
                test(r12[0].equals(cc));
                test(r13[0].equals(cc));
                test(r14[0].equals(cb));
                test(r15[0].equals(cb));
                test(r16[0].equals(cc));
                test(r17[0].equals(cc));
                test(r18[0].equals(cc));
                test(r19[0].equals(cc));
                test(r20[0].equals(cc));

                out.writeLine("ok");
                out.write("ditto, but for interface hierarchy... ");

                return Promise.all(
                    ia.iaop(ia),    // r1
                    ia.iaop(ib1),   // r2
                    ia.iaop(ib2),   // r3
                    ia.iaop(ic),    // r4
                    ib1.ib1op(ia),   // r5
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
                    ic.icop(ic));   // r36
            }
        ).then(
            function(r1, r2, r3, r4, r5, r6, r7, r8, r9, r10,
                     r11, r12, r13, r14, r15, r16, r17, r18, r19, r20,
                     r21, r22, r23, r24, r25, r26, r27, r28, r29, r30,
                     r31, r32, r33, r34, r35, r36)
            {
                test(r1[0].equals(ia));
                test(r2[0].equals(ib1));
                test(r3[0].equals(ib2));
                test(r4[0].equals(ic));
                test(r5[0].equals(ia));
                test(r6[0].equals(ib1));
                test(r7[0].equals(ib2));
                test(r8[0].equals(ic));
                test(r9[0].equals(ia));
                test(r10[0].equals(ib1));
                test(r11[0].equals(ib2));
                test(r12[0].equals(ic));
                test(r13[0].equals(ia));
                test(r14[0].equals(ib1));
                test(r15[0].equals(ib2));
                test(r16[0].equals(ic));
                test(r17[0].equals(ib1));
                test(r18[0].equals(ib1));
                test(r19[0].equals(ic));
                test(r20[0].equals(ic));
                test(r21[0].equals(ib1));
                test(r22[0].equals(ib1));
                test(r23[0].equals(ic));
                test(r24[0].equals(ic));
                test(r25[0].equals(ib2));
                test(r26[0].equals(ib2));
                test(r27[0].equals(ic));
                test(r28[0].equals(ic));
                test(r29[0].equals(ib2));
                test(r30[0].equals(ib2));
                test(r31[0].equals(ic));
                test(r32[0].equals(ic));
                test(r33[0].equals(ic));
                test(r34[0].equals(ic));
                test(r35[0].equals(ic));
                test(r36[0].equals(ic));

                out.writeLine("ok");
                out.write("ditto, but for class implementing interfaces... ");

                return Promise.all(
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
                    cd.cdop(cd));    // r12
            }
        ).then(
            function(r1, r2, r3, r4, r5, r6, r7, r8, r9, r10,
                     r11, r12)
            {
                test(r1[0].equals(cd));
                test(r2[0].equals(cd));
                test(r3[0].equals(cd));
                test(r4[0].equals(cd));
                test(r5[0].equals(cd));
                test(r6[0].equals(cd));
                test(r7[0].equals(cd));
                test(r8[0].equals(cd));
                test(r9[0].equals(cd));
                test(r10[0].equals(cd));
                test(r11[0].equals(cd));
                test(r12[0].equals(cd));
                out.writeLine("ok");
                return initial.shutdown();
            }
        ).then(
            function()
            {
                p.succeed();
            },
            function(ex)
            {
                p.fail(ex);
            }
        );
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
    exports.__test__ = run;
    exports.__clientAllTests__ = allTests;
    exports.__runServer__ = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice.__require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
