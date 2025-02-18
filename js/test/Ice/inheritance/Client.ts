// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test } from "./Test.js";
import { TestHelper } from "../../Common/TestHelper.js";

const test = TestHelper.test;

export class Client extends TestHelper {
    async allTests() {
        const communicator = this.communicator();
        const out = this.getWriter();

        const initial = new Test.InitialPrx(communicator, `initial:${this.getTestEndpoint()}`);

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

        out.write("invoking proxy operations on interface hierarchy... ");

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
