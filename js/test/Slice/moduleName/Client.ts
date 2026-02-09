// Copyright (c) ZeroC, Inc.

import { TestHelper, test } from "../../Common/TestHelper.js";
import { Test } from "./test-dep.js";
import { MyTest } from "./Test.js";

export class Client extends TestHelper {
    async run() {
        const out = this.getWriter();

        out.write("testing module names with special characters... ");
        const foo = new Test.Foo(42);
        test(foo.x === 42);

        const bar = new MyTest.Bar(foo);
        test(bar.foo.x === 42);
        out.writeLine("ok");
    }
}
