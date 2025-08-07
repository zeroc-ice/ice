// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test } from "./Test.js";
import { TestHelper, test } from "../../Common/TestHelper.js";

export class Client extends TestHelper {
    async allTests() {
        const communicator = this.communicator();
        const out = this.getWriter();

        const initial = new Test.InitialPrx(communicator, `initial:${this.getTestEndpoint()}`);

        out.write("getting proxies for interface hierarchy... ");
        const ia = await initial.iaop();
        test(ia !== null);
        const ib1 = await initial.ib1op();
        test(ib1 !== null);
        const ib2 = await initial.ib2op();
        test(ib2 !== null);
        const ic = await initial.icop();
        test(ic !== null);

        test(ia != ib1);
        test(ia != ib2);
        test(ia != ic);
        test(ib1 != ic);
        test(ib2 != ic);
        out.writeLine("ok");

        out.write("invoking proxy operations on interface hierarchy... ");

        let iao = await ia.iaop(ia);
        test(ia.equals(iao));
        iao = await ia.iaop(ib1);
        test(ib1.equals(iao));
        iao = await ia.iaop(ib2);
        test(ib2.equals(iao));
        iao = await ia.iaop(ic);
        test(ic.equals(iao));
        iao = await ib1.iaop(ia);
        test(ia.equals(iao));
        iao = await ib1.iaop(ib1);
        test(ib1.equals(iao));
        iao = await ib1.iaop(ib2);
        test(ib2.equals(iao));
        iao = await ib1.iaop(ic);
        test(ic.equals(iao));
        iao = await ib2.iaop(ia);
        test(ia.equals(iao));
        iao = await ib2.iaop(ib1);
        test(ib1.equals(iao));
        iao = await ib2.iaop(ib2);
        test(ib2.equals(iao));
        iao = await ib2.iaop(ic);
        test(ic.equals(iao));
        iao = await ic.iaop(ia);
        test(ia.equals(iao));
        iao = await ic.iaop(ib1);
        test(ib1.equals(iao));
        iao = await ic.iaop(ib2);
        test(ib2.equals(iao));
        iao = await ic.iaop(ic);
        test(ic.equals(iao));

        iao = await ib1.ib1op(ib1);
        test(ib1.equals(iao));
        let ib1o = await ib1.ib1op(ib1);
        test(ib1.equals(ib1o));
        iao = await ib1.ib1op(ic);
        test(ic.equals(iao));
        ib1o = await ib1.ib1op(ic);
        test(ic.equals(ib1o));
        iao = await ic.ib1op(ib1);
        test(ib1.equals(iao));
        ib1o = await ic.ib1op(ib1);
        test(ib1.equals(ib1o));
        iao = await ic.ib1op(ic);
        test(ic.equals(iao));
        ib1o = await ic.ib1op(ic);
        test(ic.equals(ib1o));

        iao = await ib2.ib2op(ib2);
        test(ib2.equals(iao));
        let ib2o = await ib2.ib2op(ib2);
        test(ib2.equals(ib2o));
        iao = await ib2.ib2op(ic);
        test(ic.equals(iao));
        ib2o = await ib2.ib2op(ic);
        test(ic.equals(ib2o));
        iao = await ic.ib2op(ib2);
        test(ib2.equals(iao));
        ib2o = await ic.ib2op(ib2);
        test(ib2.equals(ib2o));
        iao = await ic.ib2op(ic);
        test(ic.equals(iao));
        ib2o = await ic.ib2op(ic);
        test(ic.equals(ib2o));

        iao = await ic.icop(ic);
        test(ic.equals(iao));
        ib1o = await ic.icop(ic);
        test(ic.equals(ib1o));
        ib2o = await ic.icop(ic);
        test(ic.equals(ib2o));
        const ico = await ic.icop(ic);
        test(ico !== null);
        test(ico.equals(ic));
        out.writeLine("ok");

        await initial.shutdown();
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
