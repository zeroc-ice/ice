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
    var ArrayUtil = Ice.ArrayUtil;

    var allTests = function(out, communicator)
    {
        class PreservedI extends Test.Preserved
        {
            constructor()
            {
                super();
                ++PreservedI.counter;
            }
        }

        function PreservedFactoryI(id)
        {
            if(id === Test.Preserved.ice_staticId())
            {
                return new PreservedI();
            }
            return null;
        }

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

        var failCB = function(){ test(false); };
        var ref, base, prx;
        Ice.Promise.try(() =>
            {
                out.write("testing stringToProxy... ");
                ref = "Test:default -p 12010 -t 10000";
                base = communicator.stringToProxy(ref);
                test(base !== null);
                out.writeLine("ok");

                out.write("testing checked cast... ");
                return Test.TestIntfPrx.checkedCast(base);
            }
        ).then(obj =>
            {
                prx = obj;
                test(prx !== null);
                test(prx.equals(base));
                out.writeLine("ok");
                out.write("base as Object... ");
                return prx.SBaseAsObject();
            }
        ).then(sb =>
            {
                test(sb !== null);
                test(sb.ice_id() == "::Test::SBase");
                test(sb.sb == "SBase.sb");
                out.writeLine("ok");
                out.write("base as base... ");
                return prx.SBaseAsSBase();
            }
        ).then(sb =>
            {
                test(sb.sb == "SBase.sb");
                out.writeLine("ok");
                out.write("base with known derived as base... ");
                return prx.SBSKnownDerivedAsSBase();
            }
        ).then(sb =>
            {
                test(sb !== null);
                test(sb.sb == "SBSKnownDerived.sb");
                test(sb.sbskd == "SBSKnownDerived.sbskd");
                out.writeLine("ok");
                out.write("base with known derived as known derived... ");
                return prx.SBSKnownDerivedAsSBSKnownDerived();
            }
        ).then(sb =>
            {
                test(sb.sbskd == "SBSKnownDerived.sbskd");
                out.writeLine("ok");
                out.write("base with unknown derived as base... ");
                return prx.SBSUnknownDerivedAsSBase();
            }
        ).then(sb =>
            {
                test(sb.sb == "SBSUnknownDerived.sb");
                var p = new Ice.Promise();
                if(prx.ice_getEncodingVersion().equals(Ice.Encoding_1_0))
                {
                    return prx.SBSUnknownDerivedAsSBaseCompact().then(
                        sb => test(sb.sb == "SBSUnknownDerived.sb"),
                        ex => test(ex instanceof Ice.OperationNotExistException));
                }
                else
                {
                    return prx.SBSUnknownDerivedAsSBaseCompact().then(
                        failCB,
                        ex =>
                        {
                            test(ex instanceof Ice.OperationNotExistException ||
                                 ex instanceof Ice.NoValueFactoryException);
                        });
                }
            }
        ).then(() =>
            {
                out.writeLine("ok");
                out.write("unknown with Object as Object... ");
                return prx.SUnknownAsObject();
            }
        ).then(
            obj =>
            {
                test(!prx.ice_getEncodingVersion().equals(Ice.Encoding_1_0));
                test(obj instanceof Ice.UnknownSlicedValue);
                test(obj.getUnknownTypeId() == "::Test::SUnknown");
                return prx.checkSUnknown(obj);
            },
            ex =>
            {
                test(ex instanceof Ice.NoValueFactoryException);
                test(prx.ice_getEncodingVersion().equals(Ice.Encoding_1_0));
            }
        ).then(() =>
            {
                out.writeLine("ok");
                out.write("one-element cycle... ");
                return prx.oneElementCycle();
            }
        ).then(b =>
            {
                test(b !== null);
                test(b.ice_id() == "::Test::B");
                test(b.sb == "B1.sb");
                test(b.pb === b);
                out.writeLine("ok");
                out.write("two-element cycle... ");
                return prx.twoElementCycle();
            }
        ).then(b1 =>
            {
                test(b1 !== null);
                test(b1.ice_id() == "::Test::B");
                test(b1.sb == "B1.sb");

                var b2 = b1.pb;
                test(b2 !== null);
                test(b2.ice_id() == "::Test::B");
                test(b2.sb == "B2.sb");
                test(b2.pb == b1);

                out.writeLine("ok");
                out.write("known derived pointer slicing as derived... ");
                return prx.D1AsD1();
            }
        ).then(d1 =>
            {
                test(d1 !== null);
                test(d1.ice_id() == "::Test::D1");
                test(d1.sb == "D1.sb");
                test(d1.pb !== null);
                test(d1.pb !== d1);

                var b2 = d1.pb;
                test(b2 !== null);
                test(b2.ice_id() == "::Test::B");
                test(b2.sb == "D2.sb");
                test(b2.pb === d1);

                out.writeLine("ok");
                out.write("unknown derived pointer slicing as base... ");
                return prx.D2AsB();
            }
        ).then(b2 =>
            {
                test(b2 !== null);
                test(b2.ice_id() == "::Test::B");
                test(b2.sb == "D2.sb");
                test(b2.pb !== null);
                test(b2.pb !== b2);

                var b1 = b2.pb;
                test(b1 !== null);
                test(b1.ice_id() == "::Test::D1");
                test(b1.sb == "D1.sb");
                test(b1.pb == b2);
                test(b1.sd1 == "D1.sd1");
                test(b1.pd1 === b2);

                out.writeLine("ok");
                out.write("param ptr slicing with known first... ");
                return prx.paramTest1();
            }
        ).then(r =>
            {
                var [b1, b2] = r;
                test(b1 !== null);
                test(b1.ice_id() == "::Test::D1");
                test(b1.sb == "D1.sb");
                test(b1.pb == b2);
                test(b1 !== null);
                test(b1.sd1 == "D1.sd1");
                test(b1.pd1 === b2);

                test(b2 !== null);
                test(b2.ice_id() == "::Test::B");    // No factory, must be sliced
                test(b2.sb == "D2.sb");
                test(b2.pb === b1);
                out.writeLine("ok");
                out.write("param ptr slicing with unknown first... ");
                return prx.paramTest2();
            }
        ).then(r =>
            {
                var [b2, b1] = r;
                test(b1 !== null);
                test(b1.ice_id() == "::Test::D1");
                test(b1.sb == "D1.sb");
                test(b1.pb === b2);
                test(b1 !== null);
                test(b1.sd1 == "D1.sd1");
                test(b1.pd1 == b2);

                test(b2 !== null);
                test(b2.ice_id() == "::Test::B");    // No factory, must be sliced
                test(b2.sb == "D2.sb");
                test(b2.pb == b1);
                out.writeLine("ok");
                out.write("return value identity with known first... ");
                return prx.returnTest1();
            }
        ).then(r =>
            {
                var [ret, p1, p2] = r;
                test(ret === p1);
                out.writeLine("ok");
                out.write("return value identity with unknown first... ");
                return prx.returnTest2();
            }
        ).then(r =>
            {
                var [ret, p1, p2] = r;
                test(ret == p1);
                out.writeLine("ok");
                out.write("return value identity for input params known first... ");

                var d1 = new Test.D1();
                d1.sb = "D1.sb";
                d1.sd1 = "D1.sd1";
                var d3 = new Test.D3();
                d3.pb = d1;
                d3.sb = "D3.sb";
                d3.sd3 = "D3.sd3";
                d3.pd3 = d1;
                d1.pb = d3;
                d1.pd1 = d3;

                return prx.returnTest3(d1, d3).then(b1 =>
                    {
                        test(b1 !== null);
                        test(b1.sb == "D1.sb");
                        test(b1.ice_id() == "::Test::D1");
                        var p1 = b1;
                        test(p1 !== null);
                        test(p1.sd1 == "D1.sd1");
                        test(p1.pd1 == b1.pb);

                        var b2 = b1.pb;
                        test(b2 !== null);
                        test(b2.sb == "D3.sb");
                        test(b2.ice_id() == "::Test::B");  // Sliced by server
                        test(b2.pb == b1);

                        test(!(b2 instanceof Test.D3));
                        test(b1 !== d1);
                        test(b1 !== d3);
                        test(b2 !== d1);
                        test(b2 !== d3);
                    });
            }
        ).then(() =>
            {
                out.writeLine("ok");
                out.write("return value identity for input params unknown first... ");

                var d1 = new Test.D1();
                d1.sb = "D1.sb";
                d1.sd1 = "D1.sd1";
                var d3 = new Test.D3();
                d3.pb = d1;
                d3.sb = "D3.sb";
                d3.sd3 = "D3.sd3";
                d3.pd3 = d1;
                d1.pb = d3;
                d1.pd1 = d3;

                return prx.returnTest3(d3, d1).then(b1 =>
                    {
                        test(b1 !== null);
                        test(b1.sb == "D3.sb");
                        test(b1.ice_id() == "::Test::B");  // Sliced by server
                        test(!(b1 instanceof Test.D3));

                        var b2 = b1.pb;
                        test(b2 !== null);
                        test(b2.sb == "D1.sb");
                        test(b2.ice_id() == "::Test::D1");
                        test(b2.pb == b1);
                        var p3 = b2;
                        test(p3 !== null);
                        test(p3.sd1 == "D1.sd1");
                        test(p3.pd1 === b1);

                        test(b1 !== d1);
                        test(b1 !== d3);
                        test(b2 !== d1);
                        test(b2 !== d3);
                    });
            }
        ).then(() =>
            {
                out.writeLine("ok");
                out.write("remainder unmarshaling (3 instances)... ");
                return prx.paramTest3();
            }
        ).then(r =>
            {
                var [ret, p1, p2] = r;
                test(p1 !== null);
                test(p1.sb == "D2.sb (p1 1)");
                test(p1.pb === null);
                test(p1.ice_id() == "::Test::B");

                test(p2 !== null);
                test(p2.sb == "D2.sb (p2 1)");
                test(p2.pb === null);
                test(p2.ice_id() == "::Test::B");

                test(ret !== null);
                test(ret.sb == "D1.sb (p2 2)");
                test(ret.pb === null);
                test(ret.ice_id() == "::Test::D1");

                out.writeLine("ok");
                out.write("remainder unmarshaling (4 instances)... ");
                return prx.paramTest4();
            }
        ).then(r =>
            {
                var [ret, b] = r;
                test(b !== null);
                test(b.sb == "D4.sb (1)");
                test(b.pb === null);
                test(b.ice_id() == "::Test::B");

                test(ret !== null);
                test(ret.sb == "B.sb (2)");
                test(ret.pb === null);
                test(ret.ice_id() == "::Test::B");

                out.writeLine("ok");
                out.write("param ptr slicing, instance marshaled in unknown derived as base... ");

                var b1 = new Test.B();
                b1.sb = "B.sb(1)";
                b1.pb = b1;

                var d3 = new Test.D3();
                d3.sb = "D3.sb";
                d3.pb = d3;
                d3.sd3 = "D3.sd3";
                d3.pd3 = b1;

                var b2 = new Test.B();
                b2.sb = "B.sb(2)";
                b2.pb = b1;

                return prx.returnTest3(d3, b2);
            }
        ).then(ret =>
            {
                test(ret !== null);
                test(ret.ice_id() == "::Test::B");
                test(ret.sb == "D3.sb");
                test(ret.pb === ret);

                out.writeLine("ok");
                out.write("param ptr slicing, instance marshaled in unknown derived as derived... ");

                var d11 = new Test.D1();
                d11.sb = "D1.sb(1)";
                d11.pb = d11;
                d11.sd1 = "D1.sd1(1)";

                var d3 = new Test.D3();
                d3.sb = "D3.sb";
                d3.pb = d3;
                d3.sd3 = "D3.sd3";
                d3.pd3 = d11;

                var d12 = new Test.D1();
                d12.sb = "D1.sb(2)";
                d12.pb = d12;
                d12.sd1 = "D1.sd1(2)";
                d12.pd1 = d11;

                return prx.returnTest3(d3, d12);
            }
        ).then(ret =>
            {
                test(ret !== null);
                test(ret.ice_id() == "::Test::B");
                test(ret.sb == "D3.sb");
                test(ret.pb === ret);

                out.writeLine("ok");
                out.write("sequence slicing... ");

                var ss1b = new Test.B();
                ss1b.sb = "B.sb";
                ss1b.pb = ss1b;

                var ss1d1 = new Test.D1();
                ss1d1.sb = "D1.sb";
                ss1d1.sd1 = "D1.sd1";
                ss1d1.pb = ss1b;

                var ss1d3 = new Test.D3();
                ss1d3.sb = "D3.sb";
                ss1d3.sd3 = "D3.sd3";
                ss1d3.pb = ss1b;

                var ss2b = new Test.B();
                ss2b.sb = "B.sb";
                ss2b.pb = ss1b;

                var ss2d1 = new Test.D1();
                ss2d1.sb = "D1.sb";
                ss2d1.sd1 = "D1.sd1";
                ss2d1.pb = ss2b;

                var ss2d3 = new Test.D3();
                ss2d3.sb = "D3.sb";
                ss2d3.sd3 = "D3.sd3";
                ss2d3.pb = ss2b;

                ss1d1.pd1 = ss2b;
                ss1d3.pd3 = ss2d1;

                ss2d1.pd1 = ss1d3;
                ss2d3.pd3 = ss1d1;

                var ss1 = new Test.SS1();
                ss1.s = [];
                ss1.s[0] = ss1b;
                ss1.s[1] = ss1d1;
                ss1.s[2] = ss1d3;

                var ss2 = new Test.SS2();
                ss2.s = [];
                ss2.s[0] = ss2b;
                ss2.s[1] = ss2d1;
                ss2.s[2] = ss2d3;

                return prx.sequenceTest(ss1, ss2);
            }
        ).then(ss =>
            {
                test(ss.c1 !== null);
                var ss1b = ss.c1.s[0];
                var ss1d1 = ss.c1.s[1];
                test(ss.c2 !== null);
                var ss1d3 = ss.c1.s[2];

                test(ss.c2 !== null);
                var ss2b = ss.c2.s[0];
                var ss2d1 = ss.c2.s[1];
                var ss2d3 = ss.c2.s[2];

                test(ss1b.pb === ss1b);
                test(ss1d1.pb === ss1b);
                test(ss1d3.pb === ss1b);

                test(ss2b.pb === ss1b);
                test(ss2d1.pb === ss2b);
                test(ss2d3.pb === ss2b);

                test(ss1b.ice_id() == "::Test::B");
                test(ss1d1.ice_id() == "::Test::D1");
                test(ss1d3.ice_id() == "::Test::B");

                test(ss2b.ice_id() == "::Test::B");
                test(ss2d1.ice_id() == "::Test::D1");
                test(ss2d3.ice_id() == "::Test::B");

                out.writeLine("ok");
                out.write("dictionary slicing... ");

                var bin = new Ice.HashMap();
                for(var i = 0; i < 10; ++i)
                {
                    var s = "D1." + i;
                    var d1 = new Test.D1();
                    d1.sb = s;
                    d1.pb = d1;
                    d1.sd1 = s;
                    bin.set(i, d1);
                }
                return prx.dictionaryTest(bin);
            }
        ).then(r =>
            {
                var [ret, boutH] = r;
                var i, b, s;
                test(boutH.size === 10);
                for(i = 0; i < 10; ++i)
                {
                    b = boutH.get(i * 10);
                    test(b !== null);
                    s = "D1." + i;
                    test(b.sb == s);
                    test(b.pb !== null);
                    test(b.pb !== b);
                    test(b.pb.sb == s);
                    test(b.pb.pb == b.pb);
                }

                test(ret.size === 10);
                for(i = 0; i < 10; ++i)
                {
                    b = ret.get(i * 20);
                    test(b !== null);
                    s = "D1." + (i * 20);
                    test(b.sb == s);
                    test(b.pb === (i === 0 ? null : ret.get((i - 1) * 20)));
                    test(b.sd1 == s);
                    test(b.pd1 === b);
                }

                out.writeLine("ok");
                out.write("base exception thrown as base exception... ");
                return prx.throwBaseAsBase();
            }
        ).then(
            failCB,
            ex =>
            {
                test(ex instanceof Test.BaseException);
                test(ex.ice_id() == "::Test::BaseException");
                test(ex.sbe == "sbe");
                test(ex.pb !== null);
                test(ex.pb.sb == "sb");
                test(ex.pb.pb == ex.pb);
                out.writeLine("ok");
                out.write("derived exception thrown as base exception... ");
                return prx.throwDerivedAsBase();
            }
        ).then(
            failCB,
            ex =>
            {
                test(ex instanceof Test.DerivedException);
                test(ex.ice_id() == "::Test::DerivedException");
                test(ex.sbe == "sbe");
                test(ex.pb !== null);
                test(ex.pb.sb == "sb1");
                test(ex.pb.pb === ex.pb);
                test(ex.sde == "sde1");
                test(ex.pd1 !== null);
                test(ex.pd1.sb == "sb2");
                test(ex.pd1.pb === ex.pd1);
                test(ex.pd1.sd1 == "sd2");
                test(ex.pd1.pd1 === ex.pd1);
                out.writeLine("ok");
                out.write("derived exception thrown as derived exception... ");

                return prx.throwDerivedAsDerived();
            }
        ).then(
            failCB,
            ex =>
            {
                test(ex instanceof Test.DerivedException);
                test(ex.ice_id() == "::Test::DerivedException");
                test(ex.sbe == "sbe");
                test(ex.pb !== null);
                test(ex.pb.sb == "sb1");
                test(ex.pb.pb == ex.pb);
                test(ex.sde == "sde1");
                test(ex.pd1 !== null);
                test(ex.pd1.sb == "sb2");
                test(ex.pd1.pb === ex.pd1);
                test(ex.pd1.sd1 == "sd2");
                test(ex.pd1.pd1 === ex.pd1);

                out.writeLine("ok");
                out.write("unknown derived exception thrown as base exception... ");
                return prx.throwUnknownDerivedAsBase();
            }
        ).then(
            failCB,
            ex =>
            {
                test(ex instanceof Test.BaseException);
                test(ex.ice_id() == "::Test::BaseException");
                test(ex.sbe == "sbe");
                test(ex.pb !== null);
                test(ex.pb.sb == "sb d2");
                test(ex.pb.pb == ex.pb);

                out.writeLine("ok");
                out.write("forward-declared class... ");
                return prx.useForward();
            }
        ).then(f =>
            {
                test(f !== null);
                out.writeLine("ok");

                out.write("preserved classes... ");
                //
                // Register a factory in order to substitute our own subclass of Preserved. This provides
                // an easy way to determine how many unmarshaled instances currently exist.
                //
                // TODO: We have to install this now (even though it's not necessary yet), because otherwise
                // the Ice run time will install its own internal factory for Preserved upon receiving the
                // first instance.
                //
                communicator.getValueFactoryManager().add(PreservedFactoryI, Test.Preserved.ice_staticId());

                //
                // Server knows the most-derived class PDerived.
                //
                var pd = new Test.PDerived();
                pd.pi = 3;
                pd.ps = "preserved";
                pd.pb = pd;

                return prx.exchangePBase(pd);
            }
        ).then(r =>
            {
                var p2 = r;
                test(p2.pi === 3);
                test(p2.ps == "preserved");
                test(p2.pb === p2);

                //
                // Server only knows the base (non-preserved) type, so the object is sliced.
                //
                var pu = new Test.PCUnknown();
                pu.pi = 3;
                pu.pu = "preserved";

                return prx.exchangePBase(pu);
            }
        ).then(r =>
            {
                test(!(r instanceof Test.PCUnknown));
                test(r.pi == 3);

                //
                // Server only knows the intermediate type Preserved. The object will be sliced to
                // Preserved for the 1.0 encoding; otherwise it should be returned intact.
                //
                var pcd = new Test.PCDerived();
                pcd.pi = 3;
                pcd.pbs = [ pcd ];

                return prx.exchangePBase(pcd);
            }
        ).then(r =>
            {
                if(prx.ice_getEncodingVersion().equals(Ice.Encoding_1_0))
                {
                    test(!(r instanceof Test.PCDerived));
                    test(r.pi === 3);
                }
                else
                {
                    var p2 = r;
                    test(p2.pi === 3);
                    test(p2.pbs[0] === p2);
                }
                //
                // Server only knows the intermediate type Preserved. The object will be sliced to
                // Preserved for the 1.0 encoding; otherwise it should be returned intact.
                //
                var pcd = new Test.CompactPCDerived();
                pcd.pi = 3;
                pcd.pbs = [ pcd ];

                return prx.exchangePBase(pcd);
            }
        ).then(r =>
            {
                var p2;

                if(prx.ice_getEncodingVersion().equals(Ice.Encoding_1_0))
                {
                    test(!(r instanceof Test.CompactPCDerived));
                    test(r.pi === 3);
                }
                else
                {
                    p2 = r;
                    test(p2.pi === 3);
                    test(p2.pbs[0] === p2);
                }

                //
                // Send an object that will have multiple preserved slices in the server.
                // The object will be sliced to Preserved for the 1.0 encoding.
                //
                var pcd = new Test.PCDerived3();
                pcd.pi = 3;

                //
                // Sending more than 254 objects exercises the encoding for object ids.
                //
                pcd.pbs = new Array(300);
                for(var i = 0; i < 300; ++i)
                {
                    p2 = new Test.PCDerived2();
                    p2.pi = i;
                    p2.pbs = [ null ]; // Nil reference. This slice should not have an indirection table.
                    p2.pcd2 = i;
                    pcd.pbs[i] = p2;
                }
                pcd.pcd2 = pcd.pi;
                pcd.pcd3 = pcd.pbs[10];

                return prx.exchangePBase(pcd);
            }
        ).then(r =>
            {
                if(prx.ice_getEncodingVersion().equals(Ice.Encoding_1_0))
                {
                    test(!(r instanceof Test.PCDerived3));
                    test(r instanceof Test.Preserved);
                    test(r.pi === 3);
                }
                else
                {
                    var p3 = r;
                    test(p3.pi === 3);
                    for(var i = 0; i < 300; ++i)
                    {
                        var p2 = p3.pbs[i];
                        test(p2.pi === i);
                        test(p2.pbs.length === 1);
                        test(p2.pbs[0] === null);
                        test(p2.pcd2 === i);
                    }
                    test(p3.pcd2 === p3.pi);
                    test(p3.pcd3 === p3.pbs[10]);
                }
                //
                // Obtain an object with preserved slices and send it back to the server.
                // The preserved slices should be excluded for the 1.0 encoding, otherwise
                // they should be included.
                //
                return prx.PBSUnknownAsPreserved();
            }
        ).then(p =>
            {
                return prx.checkPBSUnknown(p).then(r =>
                    {
                        if(!prx.ice_getEncodingVersion().equals(Ice.Encoding_1_0))
                        {
                            return prx.ice_encodingVersion(Ice.Encoding_1_0).checkPBSUnknown(p);
                        }
                    });
            }
        ).then(() =>
            {
                out.writeLine("ok");
                return prx.shutdown();
            }
        ).then(p.resolve, p.reject);
        return p;
    };

    var run = function(out, id)
    {
        return Ice.Promise.try(
            function()
            {
                var c = Ice.initialize(id);
                return allTests(out, c).finally(() =>
                    {
                        if(c)
                        {
                            return c.destroy();
                        }
                    });
            });
    };

    exports._test = run;
    exports._runServer = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
