// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import {Ice} from "ice";
import {Test, Inner} from "./generated";
import {TestHelper} from "../../../Common/TestHelper";

const test = TestHelper.test;

export class Client extends TestHelper
{
    async allTests()
    {
        const out = this.getWriter();
        const communicator = this.communicator();

        out.write("test same Slice type name in different scopes... ");

        {
            const i1 = await Test.IPrx.checkedCast(communicator.stringToProxy("i1:" + this.getTestEndpoint()));
            const s1 = new Test.S(0);

            const [s2, s3] = await i1.opS(s1);
            test(s2.equals(s1));
            test(s3.equals(s1));

            const [sseq2, sseq3] = await i1.opSSeq([s1]);
            test(sseq2[0].equals(s1));
            test(sseq3[0].equals(s1));

            const smap1 = new Map([["a", s1]]);
            const [smap2, smap3] = await i1.opSMap(smap1);
            test(smap2.get("a").equals(s1));
            test(smap3.get("a").equals(s1));

            const c1 = new Test.C(s1);

            const [c2, c3] = await i1.opC(c1);
            test(c2.s.equals(s1));
            test(c3.s.equals(s1));

            const [cseq2, cseq3] = await i1.opCSeq([c1]);
            test(cseq2[0].s.equals(s1));
            test(cseq3[0].s.equals(s1));

            const cmap1 = new Map([["a", c1]]);
            const [cmap2, cmap3] = await i1.opCMap(cmap1);
            test(cmap2.get("a").s.equals(s1));
            test(cmap3.get("a").s.equals(s1));
        }

        {
            const i2 = await Test.Inner.Inner2.IPrx.checkedCast(
                communicator.stringToProxy("i2:" + this.getTestEndpoint()));
            const s1 = new Test.Inner.Inner2.S(0);

            const [s2, s3] = await i2.opS(s1);
            test(s2.equals(s1));
            test(s3.equals(s1));

            const [sseq2, sseq3] = await i2.opSSeq([s1]);
            test(sseq2[0].equals(s1));
            test(sseq3[0].equals(s1));

            const smap1 = new Map([["a", s1]]);
            const [smap2, smap3] = await i2.opSMap(smap1);
            test(smap2.get("a").equals(s1));
            test(smap3.get("a").equals(s1));

            const c1 = new Test.Inner.Inner2.C(s1);

            const [c2, c3] = await i2.opC(c1);
            test(c2.s.equals(s1));
            test(c3.s.equals(s1));

            const [cseq2, cseq3] = await i2.opCSeq([c1]);
            test(cseq2[0].s.equals(s1));
            test(cseq3[0].s.equals(s1));

            const cmap1 = new Map([["a", c1]]);
            const [cmap2, cmap3] = await i2.opCMap(cmap1);
            test(cmap2.get("a").s.equals(s1));
            test(cmap3.get("a").s.equals(s1));
        }

        {
            const i3 = await Test.Inner.IPrx.checkedCast(
                communicator.stringToProxy("i3:" + this.getTestEndpoint()));
            const s1 = new Test.Inner.Inner2.S(0);

            const [s2, s3] = await i3.opS(s1);
            test(s2.equals(s1));
            test(s3.equals(s1));

            const [sseq2, sseq3] = await i3.opSSeq([s1]);
            test(sseq2[0].equals(s1));
            test(sseq3[0].equals(s1));

            const smap1 = new Map([["a", s1]]);
            const [smap2, smap3] = await i3.opSMap(smap1);
            test(smap2.get("a").equals(s1));
            test(smap3.get("a").equals(s1));

            const c1 = new Test.Inner.Inner2.C(s1);

            const [c2, c3] = await i3.opC(c1);
            test(c2.s.equals(s1));
            test(c3.s.equals(s1));

            const [cseq2, cseq3] = await i3.opCSeq([c1]);
            test(cseq2[0].s.equals(s1));
            test(cseq3[0].s.equals(s1));

            const cmap1 = new Map([["a", c1]]);
            const [cmap2, cmap3] = await i3.opCMap(cmap1);
            test(cmap2.get("a").s.equals(s1));
            test(cmap3.get("a").s.equals(s1));
        }

        {
            const i4 = await Inner.Test.Inner2.IPrx.checkedCast(
                communicator.stringToProxy("i4:" + this.getTestEndpoint()));
            const s1 = new Test.S(0);

            const [s2, s3] = await i4.opS(s1);
            test(s2.equals(s1));
            test(s3.equals(s1));

            const [sseq2, sseq3] = await i4.opSSeq([s1]);
            test(sseq2[0].equals(s1));
            test(sseq3[0].equals(s1));

            const smap1 = new Map([["a", s1]]);
            const [smap2, smap3] = await i4.opSMap(smap1);
            test(smap2.get("a").equals(s1));
            test(smap3.get("a").equals(s1));

            const c1 = new Test.C(s1);

            const [c2, c3] = await i4.opC(c1);
            test(c2.s.equals(s1));
            test(c3.s.equals(s1));

            const [cseq2, cseq3] = await i4.opCSeq([c1]);
            test(cseq2[0].s.equals(s1));
            test(cseq3[0].s.equals(s1));

            const cmap1 = new Map([["a", c1]]);
            const [cmap2, cmap3] = await i4.opCMap(cmap1);
            test(cmap2.get("a").s.equals(s1));
            test(cmap3.get("a").s.equals(s1));
        }

        {
            const i1 = await Test.IPrx.checkedCast(communicator.stringToProxy("i1:" + this.getTestEndpoint()));
            await i1.shutdown();
        }

        out.writeLine("ok");
    }

    async run(args:string[])
    {
        let x;
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
