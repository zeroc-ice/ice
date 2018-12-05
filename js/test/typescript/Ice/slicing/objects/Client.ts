// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import {Ice} from "ice";
import {Test} from "./generated";
import {TestHelper} from "../../../../Common/TestHelper";

const test = TestHelper.test;

export class Client extends TestHelper
{
    async allTests()
    {
        class PreservedI extends Test.Preserved
        {
            constructor()
            {
                super();
                ++PreservedI.counter;
            }

            static counter:number;
        }

        function PreservedFactoryI(id:string)
        {
            return id === Test.Preserved.ice_staticId() ? new PreservedI() : null;
        }

        const out = this.getWriter();
        const communicator = this.communicator();

        out.write("testing stringToProxy... ");
        const ref = "Test:" + this.getTestEndpoint() + " -t 10000";
        const base = communicator.stringToProxy(ref);
        test(base !== null);
        out.writeLine("ok");

        out.write("testing checked cast... ");
        const prx = await Test.TestIntfPrx.checkedCast(base);
        test(prx !== null);
        test(prx.equals(base));
        out.writeLine("ok");

        out.write("base as Object... ");
        {
            const sb = await prx.SBaseAsObject() as Test.SBase;
            test(sb !== null);
            test(sb.ice_id() == "::Test::SBase");
            test(sb.sb == "SBase.sb");
        }
        out.writeLine("ok");

        out.write("base as base... ");
        {
            const sb = await prx.SBaseAsSBase();
            test(sb.sb == "SBase.sb");
        }
        out.writeLine("ok");

        out.write("base with known derived as base... ");
        {
            const sb = await prx.SBSKnownDerivedAsSBase() as Test.SBSKnownDerived;
            test(sb !== null);
            test(sb.sb == "SBSKnownDerived.sb");
            test(sb.sbskd == "SBSKnownDerived.sbskd");
        }
        out.writeLine("ok");

        out.write("base with known derived as known derived... ");
        {
            const sb = await prx.SBSKnownDerivedAsSBSKnownDerived();
            test(sb.sbskd == "SBSKnownDerived.sbskd");
        }
        out.writeLine("ok");

        out.write("base with unknown derived as base... ");
        {
            const sb = await prx.SBSUnknownDerivedAsSBase();
            test(sb.sb == "SBSUnknownDerived.sb");
        }

        if(prx.ice_getEncodingVersion().equals(Ice.Encoding_1_0))
        {
            try
            {
                const sb = await prx.SBSUnknownDerivedAsSBaseCompact();
                test(sb.sb == "SBSUnknownDerived.sb");
            }
            catch(ex)
            {
                test(ex instanceof Ice.OperationNotExistException, ex);
            }
        }
        else
        {
            try
            {
                await prx.SBSUnknownDerivedAsSBaseCompact();
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.OperationNotExistException ||
                     ex instanceof Ice.NoValueFactoryException, ex);
            }
        }
        out.writeLine("ok");

        out.write("unknown with Object as Object... ");
        try
        {
            const obj = await prx.SUnknownAsObject();
            test(!prx.ice_getEncodingVersion().equals(Ice.Encoding_1_0));
            test(obj instanceof Ice.UnknownSlicedValue);
            test(obj.ice_id() == "::Test::SUnknown");
            test(obj.ice_getSlicedData() !== null);
            await prx.checkSUnknown(obj);
        }
        catch(ex)
        {
            test(ex instanceof Ice.NoValueFactoryException, ex);
            test(prx.ice_getEncodingVersion().equals(Ice.Encoding_1_0));
        }
        out.writeLine("ok");

        out.write("one-element cycle... ");
        {
            const b = await prx.oneElementCycle();
            test(b !== null);
            test(b.ice_id() == "::Test::B");
            test(b.sb == "B1.sb");
            test(b.pb === b);
        }
        out.writeLine("ok");

        out.write("two-element cycle... ");
        {
            const b1 = await prx.twoElementCycle();
            test(b1 !== null);
            test(b1.ice_id() == "::Test::B");
            test(b1.sb == "B1.sb");

            const b2 = b1.pb;
            test(b2 !== null);
            test(b2.ice_id() == "::Test::B");
            test(b2.sb == "B2.sb");
            test(b2.pb == b1);
        }
        out.writeLine("ok");

        out.write("known derived pointer slicing as derived... ");
        {
            const d1 = await prx.D1AsD1();

            test(d1 !== null);
            test(d1.ice_id() == "::Test::D1");
            test(d1.sb == "D1.sb");
            test(d1.pb !== null);
            test(d1.pb !== d1);

            const b2 = d1.pb;
            test(b2 !== null);
            test(b2.ice_id() == "::Test::B");
            test(b2.sb == "D2.sb");
            test(b2.pb === d1);
        }
        out.writeLine("ok");

        out.write("unknown derived pointer slicing as base... ");
        {
            const b2 = await prx.D2AsB() as Test.B;

            test(b2 !== null);
            test(b2.ice_id() == "::Test::B");
            test(b2.sb == "D2.sb");
            test(b2.pb !== null);
            test(b2.pb !== b2);

            const b1 = b2.pb;
            test(b1 !== null);
            test(b1.ice_id() == "::Test::D1");
            test(b1.sb == "D1.sb");
            test(b1.pb == b2);
            const d1 = b1 as Test.D1;
            test(d1.sd1 == "D1.sd1");
            test(d1.pd1 === b2);
        }
        out.writeLine("ok");

        out.write("param ptr slicing with known first... ");
        {
            const [b1, b2] = await prx.paramTest1();
            test(b1 !== null);
            test(b1.ice_id() == "::Test::D1");
            test(b1.sb == "D1.sb");
            test(b1.pb == b2);
            test(b1 !== null);
            const d1:Test.D1 = b1 as Test.D1;
            test(d1.sd1 == "D1.sd1");
            test(d1.pd1 === b2);

            test(b2 !== null);
            test(b2.ice_id() == "::Test::B"); // No factory, must be sliced
            test(b2.sb == "D2.sb");
            test(b2.pb === b1);
        }
        out.writeLine("ok");

        out.write("param ptr slicing with unknown first... ");
        {
            const [b2, b1] = await prx.paramTest2();
            test(b1 !== null);
            test(b1.ice_id() == "::Test::D1");
            test(b1.sb == "D1.sb");
            test(b1.pb === b2);
            test(b1 !== null);
            const d1:Test.D1 = b1 as Test.D1;
            test(d1.sd1 == "D1.sd1");
            test(d1.pd1 == b2);

            test(b2 !== null);
            test(b2.ice_id() == "::Test::B"); // No factory, must be sliced
            test(b2.sb == "D2.sb");
            test(b2.pb == b1);
        }
        out.writeLine("ok");

        out.write("return value identity with known first... ");
        {
            const [ret, p1] = await prx.returnTest1();
            test(ret === p1);
        }
        out.writeLine("ok");

        out.write("return value identity with unknown first... ");
        {
            const [ret, p1] = await prx.returnTest2();
            test(ret == p1);
        }
        out.writeLine("ok");

        out.write("return value identity for input params known first... ");
        {
            const d1 = new Test.D1();
            d1.sb = "D1.sb";
            d1.sd1 = "D1.sd1";
            const d3 = new Test.D3();
            d3.pb = d1;
            d3.sb = "D3.sb";
            d3.sd3 = "D3.sd3";
            d3.pd3 = d1;
            d1.pb = d3;
            d1.pd1 = d3;

            const b1 = await prx.returnTest3(d1, d3);
            test(b1 !== null);
            test(b1.sb == "D1.sb");
            test(b1.ice_id() == "::Test::D1");
            const p1 = b1 as Test.D1;
            test(p1 !== null);
            test(p1.sd1 == "D1.sd1");
            test(p1.pd1 == b1.pb);

            const b2 = b1.pb;
            test(b2 !== null);
            test(b2.sb == "D3.sb");
            test(b2.ice_id() == "::Test::B"); // Sliced by server
            test(b2.pb == b1);

            test(!(b2 instanceof Test.D3));
            test(b1 !== d1);
            test(b1 !== d3);
            test(b2 !== d1);
            test(b2 !== d3);
        }
        out.writeLine("ok");

        out.write("return value identity for input params unknown first... ");
        {
            const d1 = new Test.D1();
            d1.sb = "D1.sb";
            d1.sd1 = "D1.sd1";
            const d3 = new Test.D3();
            d3.pb = d1;
            d3.sb = "D3.sb";
            d3.sd3 = "D3.sd3";
            d3.pd3 = d1;
            d1.pb = d3;
            d1.pd1 = d3;

            const b1 = await prx.returnTest3(d3, d1);

            test(b1 !== null);
            test(b1.sb == "D3.sb");
            test(b1.ice_id() == "::Test::B"); // Sliced by server
            test(!(b1 instanceof Test.D3));

            const b2 = b1.pb;
            test(b2 !== null);
            test(b2.sb == "D1.sb");
            test(b2.ice_id() == "::Test::D1");
            test(b2.pb == b1);

            const p3:Test.D1 = b2 as Test.D1;
            test(p3 !== null);
            test(p3.sd1 == "D1.sd1");
            test(p3.pd1 === b1);

            test(b1 !== d1);
            test(b1 !== d3);
            test(b2 !== d1);
            test(b2 !== d3);
        }
        out.writeLine("ok");

        out.write("remainder unmarshaling (3 instances)... ");
        {
            const [ret, p1, p2] = await prx.paramTest3();
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
        }
        out.writeLine("ok");

        out.write("remainder unmarshaling (4 instances)... ");
        {
            const [ret, b] = await prx.paramTest4();
            test(b !== null);
            test(b.sb == "D4.sb (1)");
            test(b.pb === null);
            test(b.ice_id() == "::Test::B");

            test(ret !== null);
            test(ret.sb == "B.sb (2)");
            test(ret.pb === null);
            test(ret.ice_id() == "::Test::B");
        }
        out.writeLine("ok");

        out.write("param ptr slicing, instance marshaled in unknown derived as base... ");
        {
            const b1 = new Test.B();
            b1.sb = "B.sb(1)";
            b1.pb = b1;

            const d3 = new Test.D3();
            d3.sb = "D3.sb";
            d3.pb = d3;
            d3.sd3 = "D3.sd3";
            d3.pd3 = b1;

            const b2 = new Test.B();
            b2.sb = "B.sb(2)";
            b2.pb = b1;

            const ret = await prx.returnTest3(d3, b2);
            test(ret !== null);
            test(ret.ice_id() == "::Test::B");
            test(ret.sb == "D3.sb");
            test(ret.pb === ret);
        }
        out.writeLine("ok");

        out.write("param ptr slicing, instance marshaled in unknown derived as derived... ");
        {
            const d11 = new Test.D1();
            d11.sb = "D1.sb(1)";
            d11.pb = d11;
            d11.sd1 = "D1.sd1(1)";

            const d3 = new Test.D3();
            d3.sb = "D3.sb";
            d3.pb = d3;
            d3.sd3 = "D3.sd3";
            d3.pd3 = d11;

            const d12 = new Test.D1();
            d12.sb = "D1.sb(2)";
            d12.pb = d12;
            d12.sd1 = "D1.sd1(2)";
            d12.pd1 = d11;

            const ret = await prx.returnTest3(d3, d12);
            test(ret !== null);
            test(ret.ice_id() == "::Test::B");
            test(ret.sb == "D3.sb");
            test(ret.pb === ret);
        }
        out.writeLine("ok");

        out.write("sequence slicing... ");
        {
            let ss1b = new Test.B();
            ss1b.sb = "B.sb";
            ss1b.pb = ss1b;

            let ss1d1 = new Test.D1();
            ss1d1.sb = "D1.sb";
            ss1d1.sd1 = "D1.sd1";
            ss1d1.pb = ss1b;

            let ss1d3 = new Test.D3();
            ss1d3.sb = "D3.sb";
            ss1d3.sd3 = "D3.sd3";
            ss1d3.pb = ss1b;

            let ss2b = new Test.B();
            ss2b.sb = "B.sb";
            ss2b.pb = ss1b;

            let ss2d1 = new Test.D1();
            ss2d1.sb = "D1.sb";
            ss2d1.sd1 = "D1.sd1";
            ss2d1.pb = ss2b;

            let ss2d3 = new Test.D3();
            ss2d3.sb = "D3.sb";
            ss2d3.sd3 = "D3.sd3";
            ss2d3.pb = ss2b;

            ss1d1.pd1 = ss2b;
            ss1d3.pd3 = ss2d1;

            ss2d1.pd1 = ss1d3;
            ss2d3.pd3 = ss1d1;

            const ss1 = new Test.SS1();
            ss1.s = [];
            ss1.s[0] = ss1b;
            ss1.s[1] = ss1d1;
            ss1.s[2] = ss1d3;

            const ss2 = new Test.SS2();
            ss2.s = [];
            ss2.s[0] = ss2b;
            ss2.s[1] = ss2d1;
            ss2.s[2] = ss2d3;

            const ss = await prx.sequenceTest(ss1, ss2);

            test(ss.c1 !== null);
            const ss1b2 = ss.c1.s[0];
            const ss1d2 = ss.c1.s[1];
            test(ss.c2 !== null);
            const ss1d4 = ss.c1.s[2];

            test(ss.c2 !== null);
            const ss2b2 = ss.c2.s[0];
            const ss2d2 = ss.c2.s[1];
            const ss2d4 = ss.c2.s[2];

            test(ss1b2.pb == ss1b2);
            test(ss1d2.pb == ss1b2);
            test(ss1d4.pb == ss1b2);

            test(ss2b2.pb == ss1b2);
            test(ss2d2.pb == ss2b2);
            test(ss2d4.pb == ss2b2);

            test(ss1b2.ice_id() == "::Test::B");
            test(ss1d2.ice_id() == "::Test::D1");
            test(ss1d4.ice_id() == "::Test::B");

            test(ss2b2.ice_id() == "::Test::B");
            test(ss2d2.ice_id() == "::Test::D1");
            test(ss2d4.ice_id() == "::Test::B");
        }
        out.writeLine("ok");

        out.write("dictionary slicing... ");
        {
            const bin = new Map();
            for(let i = 0; i < 10; ++i)
            {
                const s = `D1.${i}`;
                const d1 = new Test.D1();
                d1.sb = s;
                d1.pb = d1;
                d1.sd1 = s;
                bin.set(i, d1);
            }

            const [ret, boutH] = await prx.dictionaryTest(bin);
            test(boutH.size === 10);
            for(let i = 0; i < 10; ++i)
            {
                const b = boutH.get(i * 10);
                test(b !== null);
                const s = `D1.${i}`;
                test(b.sb == s);
                test(b.pb !== null);
                test(b.pb !== b);
                test(b.pb.sb == s);
                test(b.pb.pb == b.pb);
            }

            test(ret.size === 10);
            for(let i = 0; i < 10; ++i)
            {
                const b = ret.get(i * 20);
                test(b !== null);
                const s = "D1." + (i * 20);
                test(b.sb == s);
                test(b.pb === (i === 0 ? null : ret.get((i - 1) * 20)));
                const d1 = b as Test.D1;
                test(d1 != null);
                test(d1.sd1 == s);
                test(d1.pd1 === b);
            }
        }
        out.writeLine("ok");

        out.write("base exception thrown as base exception... ");
        try
        {
            await prx.throwBaseAsBase();
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Test.BaseException, ex);
            test(ex.ice_id() == "::Test::BaseException");
            test(ex.sbe == "sbe");
            test(ex.pb !== null);
            test(ex.pb.sb == "sb");
            test(ex.pb.pb == ex.pb);
        }
        out.writeLine("ok");

        out.write("derived exception thrown as base exception... ");
        try
        {
            await prx.throwDerivedAsBase();
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Test.DerivedException, ex);
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
        }
        out.writeLine("ok");

        out.write("derived exception thrown as derived exception... ");
        try
        {
            await prx.throwDerivedAsDerived();
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Test.DerivedException, ex);
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
        }
        out.writeLine("ok");

        out.write("unknown derived exception thrown as base exception... ");
        try
        {
            await prx.throwUnknownDerivedAsBase();
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Test.BaseException, ex);
            test(ex.ice_id() == "::Test::BaseException");
            test(ex.sbe == "sbe");
            test(ex.pb !== null);
            test(ex.pb.sb == "sb d2");
            test(ex.pb.pb == ex.pb);
        }
        out.writeLine("ok");

        out.write("forward-declared class... ");
        {
            const f = await prx.useForward();
            test(f !== null);
        }
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
        {
            //
            // Server knows the most-derived class PDerived.
            //
            const pd = new Test.PDerived();
            pd.pi = 3;
            pd.ps = "preserved";
            pd.pb = pd;

            const p2 = await prx.exchangePBase(pd) as Test.PDerived;

            test(p2.pi === 3);
            test(p2.ps == "preserved");
            test(p2.pb === p2);
        }

        {
            //
            // Server only knows the base (non-preserved) type, so the object is sliced.
            //
            const pu = new Test.PCUnknown();
            pu.pi = 3;
            pu.pu = "preserved";

            const r = await prx.exchangePBase(pu);

            test(!(r instanceof Test.PCUnknown));
            test(r.pi == 3);
        }

        {
            //
            // Server only knows the intermediate type Preserved. The object will be sliced to
            // Preserved for the 1.0 encoding; otherwise it should be returned intact.
            //
            const pcd = new Test.PCDerived();
            pcd.pi = 3;
            pcd.pbs = [pcd];

            const r = await prx.exchangePBase(pcd);
            if(prx.ice_getEncodingVersion().equals(Ice.Encoding_1_0))
            {
                test(!(r instanceof Test.PCDerived));
                test(r.pi === 3);
            }
            else
            {
                const p2 = r as Test.PCDerived;
                test(p2.pi === 3);
                test(p2.pbs[0] === r);
            }
        }

        {
            //
            // Server only knows the intermediate type Preserved. The object will be sliced to
            // Preserved for the 1.0 encoding; otherwise it should be returned intact.
            //
            const pcd = new Test.CompactPCDerived();
            pcd.pi = 3;
            pcd.pbs = [pcd];

            const r = await prx.exchangePBase(pcd);
            if(prx.ice_getEncodingVersion().equals(Ice.Encoding_1_0))
            {
                test(!(r instanceof Test.CompactPCDerived));
                test(r.pi === 3);
            }
            else
            {
                const p2 = r as Test.PCDerived;
                test(p2.pi === 3);
                test(p2.pbs[0] === p2);
            }
        }

        {
            //
            // Send an object that will have multiple preserved slices in the server.
            // The object will be sliced to Preserved for the 1.0 encoding.
            //
            const pcd = new Test.PCDerived3();
            pcd.pi = 3;

            //
            // Sending more than 254 objects exercises the encoding for object ids.
            //
            pcd.pbs = new Array(300);
            for(let i = 0; i < 300; ++i)
            {
                const p2 = new Test.PCDerived2();
                p2.pi = i;
                p2.pbs = [null]; // Nil reference. This slice should not have an indirection table.
                p2.pcd2 = i;
                pcd.pbs[i] = p2;
            }
            pcd.pcd2 = pcd.pi;
            pcd.pcd3 = pcd.pbs[10];

            const r = await prx.exchangePBase(pcd);
            if(prx.ice_getEncodingVersion().equals(Ice.Encoding_1_0))
            {
                test(!(r instanceof Test.PCDerived3));
                test(r instanceof Test.Preserved);
                test(r.pi === 3);
            }
            else
            {
                const p3 = r as Test.PCDerived3;
                test(p3.pi === 3);
                for(let i = 0; i < 300; ++i)
                {
                    const p2 = p3.pbs[i] as Test.PCDerived2;
                    test(p2.pi === i);
                    test(p2.pbs.length === 1);
                    test(p2.pbs[0] === null);
                    test(p2.pcd2 === i);
                }
                test(p3.pcd2 === p3.pi);
                test(p3.pcd3 === p3.pbs[10]);
            }
        }

        {
            //
            // Obtain an object with preserved slices and send it back to the server.
            // The preserved slices should be excluded for the 1.0 encoding, otherwise
            // they should be included.
            //
            const p = await prx.PBSUnknownAsPreserved();
            await prx.checkPBSUnknown(p);

            if(!prx.ice_getEncodingVersion().equals(Ice.Encoding_1_0))
            {
                const slicedData = p.ice_getSlicedData();
                test(slicedData !== null);
                test(slicedData.slices.length === 1);
                test(slicedData.slices[0].typeId == "::Test::PSUnknown");
                await prx.ice_encodingVersion(Ice.Encoding_1_0).checkPBSUnknown(p);
            }
            else
            {
                test(p.ice_getSlicedData() === null);
            }
        }
        out.writeLine("ok");
        await prx.shutdown();
    }

    async run(args:string[])
    {
        let communicator:Ice.Communicator;
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
