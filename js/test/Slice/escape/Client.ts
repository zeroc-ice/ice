// Copyright (c) ZeroC, Inc.

import { Ice } from "ice";
import { TestHelper } from "../../Common/TestHelper.js";
import { _await } from "./Key.js";

const test = TestHelper.test;

export class Client extends TestHelper {
    async allTests() {
        const communicator = this.communicator();
        const out = this.getWriter();

        out.write("testing enums... ");

        test(_await._var.base.value === 0);
        out.writeLine("ok");

        out.write("testing structs... ");
        const s = new _await._break(10);
        test(s._while == 10);
        out.writeLine("ok");

        out.write("testing proxies... ");

        const casePrx = new _await.casePrx(communicator, `hello: ${this.getTestEndpoint()}`);

        try {
            await casePrx._catch(10);
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.LocalException);
        }

        const typeofPrx = new _await.typeofPrx(communicator, `hello: ${this.getTestEndpoint()}`).ice_invocationTimeout(
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

        const d = new _await._delete(10, null, 10);
        test(d._if === 10);
        test(d._else === null);
        test(d._export === 10);

        let p = new _await._package(
            new _await._break(100),
            _await._var.base,
            _await.explicitPrx.uncheckedCast(communicator.stringToProxy("hello")),
            new Map<string, _await._break>(),
            "",
            _await.explicitPrx.uncheckedCast(communicator.stringToProxy("hello")),
        );

        test(p._for._while === 100);
        test(p.goto === _await._var.base);
        test(p._if instanceof _await.explicitPrx);
        test(p.internal instanceof Map);
        test(p._debugger === "");
        test(p._null instanceof _await.explicitPrx);
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
