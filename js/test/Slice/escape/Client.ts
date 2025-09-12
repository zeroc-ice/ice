// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { TestHelper, test } from "../../Common/TestHelper.js";
import { escapedAwait } from "./Key.js";
export class Client extends TestHelper {
    async allTests() {
        const communicator = this.communicator();
        const out = this.getWriter();

        out.write("testing enums... ");

        test(escapedAwait._var.base.value === 0);
        out.writeLine("ok");

        out.write("testing structs... ");
        const s = new escapedAwait._break(10);
        test(s._while == 10);
        out.writeLine("ok");

        out.write("testing proxies... ");

        const casePrx = new escapedAwait._casePrx(communicator, `hello: ${this.getTestEndpoint()}`);

        try {
            await casePrx._catch(10);
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.LocalException);
        }

        const typeofPrx = new escapedAwait._typeofPrx(communicator, `hello: ${this.getTestEndpoint()}`).ice_invocationTimeout(
            100,
        );

        try {
            await typeofPrx._default();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.LocalException);
        }
        out.writeLine("ok");

        out.write("testing classes... ");

        const d = new escapedAwait._delete(null!, 10);
        test(d._else === null);
        test(d._export === 10);

        const p = new escapedAwait._package(
            new escapedAwait._break(100),
            escapedAwait._var.base,
            new escapedAwait.explicitPrx(communicator, "hello"),
            new Map<string, escapedAwait._break>(),
            "",
        );

        test(p._for!._while === 100);
        test(p.goto === escapedAwait._var.base);
        test(p.internal instanceof Map);
        test(p._debugger === "");
        test(p._null instanceof escapedAwait.explicitPrx);
        out.writeLine("ok");
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
