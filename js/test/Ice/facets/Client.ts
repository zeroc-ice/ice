// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test } from "./Test.js";
import { TestHelper } from "../../Common/TestHelper.js";

const test = TestHelper.test;

export class Client extends TestHelper {
    async allTests() {
        const communicator = this.communicator();
        const out = this.getWriter();

        const db = new Ice.ObjectPrx(communicator, `d:${this.getTestEndpoint()}`);

        out.write("testing unchecked cast... ");
        let prx = Ice.ObjectPrx.uncheckedCast(db);
        test(prx.ice_getFacet().length === 0);
        prx = Ice.ObjectPrx.uncheckedCast(db, "facetABCD");
        test(prx.ice_getFacet() == "facetABCD");
        let prx2 = Ice.ObjectPrx.uncheckedCast(prx);
        test(prx2.ice_getFacet() == "facetABCD");
        let prx3 = Ice.ObjectPrx.uncheckedCast(prx, "");
        test(prx3.ice_getFacet().length === 0);
        let d = Test.DPrx.uncheckedCast(db);
        test(d.ice_getFacet().length === 0);
        let df = Test.DPrx.uncheckedCast(db, "facetABCD");
        test(df.ice_getFacet() == "facetABCD");
        let df2 = Test.DPrx.uncheckedCast(df);
        test(df2.ice_getFacet() == "facetABCD");
        let df3 = Test.DPrx.uncheckedCast(df, "");
        test(df3.ice_getFacet().length === 0);
        out.writeLine("ok");

        out.write("testing checked cast... ");
        prx = (await Ice.ObjectPrx.checkedCast(db)) as Ice.ObjectPrx;
        test(prx.ice_getFacet().length === 0);
        prx = (await Ice.ObjectPrx.checkedCast(db, "facetABCD")) as Ice.ObjectPrx;
        test(prx.ice_getFacet() == "facetABCD");
        prx2 = (await Ice.ObjectPrx.checkedCast(prx)) as Ice.ObjectPrx;
        test(prx2.ice_getFacet() == "facetABCD");
        prx3 = (await Ice.ObjectPrx.checkedCast(prx, "")) as Ice.ObjectPrx;
        test(prx3.ice_getFacet().length === 0);
        d = (await Test.DPrx.checkedCast(db)) as Test.DPrx;
        test(d.ice_getFacet().length === 0);
        df = (await Test.DPrx.checkedCast(db, "facetABCD")) as Test.DPrx;
        test(df.ice_getFacet() == "facetABCD");
        df2 = (await Test.DPrx.checkedCast(df)) as Test.DPrx;
        test(df2.ice_getFacet() == "facetABCD");
        df3 = (await Test.DPrx.checkedCast(df, "")) as Test.DPrx;
        test(df3.ice_getFacet().length === 0);
        out.writeLine("ok");

        out.write("testing non-facets A, B, C, and D... ");
        d = (await Test.DPrx.checkedCast(db)) as Test.DPrx;
        test(d !== null);
        test(d.equals(db));
        test((await d.callA()) == "A");
        test((await d.callB()) == "B");
        test((await d.callC()) == "C");
        test((await d.callD()) == "D");
        out.writeLine("ok");

        out.write("testing facets A, B, C, and D... ");
        df = (await Test.DPrx.checkedCast(d, "facetABCD")) as Test.DPrx;
        test(df !== null);
        test((await df.callA()) == "A");
        test((await df.callB()) == "B");
        test((await df.callC()) == "C");
        test((await df.callD()) == "D");
        out.writeLine("ok");

        out.write("testing facets E and F... ");
        const ff = (await Test.FPrx.checkedCast(d, "facetEF")) as Test.FPrx;
        test((await ff.callE()) == "E");
        test((await ff.callF()) == "F");
        out.writeLine("ok");

        out.write("testing facet G... ");
        const gf = (await Test.GPrx.checkedCast(ff, "facetGH")) as Test.GPrx;
        test(gf !== null);
        test((await gf.callG()) == "G");
        out.writeLine("ok");

        out.write("testing whether casting preserves the facet... ");
        const hf = (await Test.HPrx.checkedCast(gf)) as Test.HPrx;
        test(hf !== null);
        test((await hf.callG()) == "G");
        test((await hf.callH()) == "H");
        out.writeLine("ok");

        await gf.shutdown();
    }

    async run(args: string[]) {
        let communicator: Ice.Communicator | null = null;
        try {
            [communicator] = this.initialize(args);
            await this.allTests();
        } finally {
            if (communicator) {
                await communicator.destroy();
            }
        }
    }
}
