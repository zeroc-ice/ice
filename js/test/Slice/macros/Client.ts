// Copyright (c) ZeroC, Inc.

import { Ice } from "ice";
import { Test } from "./Test.js";
import { TestHelper } from "../../Common/TestHelper.js";

const test = TestHelper.test;

export class Client extends TestHelper {
    allTests() {
        const out = this.getWriter();
        out.write("testing Slice predefined macros... ");

        const d = new Test.Default();
        test(d.x == 10);
        test(d.y == 10);

        const nd = new Test.NoDefault();
        test(nd.x != 10);
        test(nd.y != 10);

        const c = new Test.JsOnly();
        test(c.lang == "js");
        test(c.version == Ice.intVersion());

        out.writeLine("ok");
    }

    run(args: string[]) {
        this.allTests();
    }
}
