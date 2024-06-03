//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("ice").Ice;
const Test = require("Test").Test;
const TestHelper = require("TestHelper").TestHelper;
const test = TestHelper.test;

class Client extends TestHelper
{
    allTests()
    {
        const out = this.getWriter();
        out.write("testing Slice predefined macros... ");

        const d = new Test._Default();
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

    run(args)
    {
        this.allTests();
    }
}
exports.Client = Client;
