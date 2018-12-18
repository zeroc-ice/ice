// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

import {Ice} from "ice";
import {Test} from "./generated";
import {TestHelper} from "../../../Common/TestHelper";

const test = TestHelper.test;

export class Client extends TestHelper
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

    run(args:string[])
    {
        this.allTests();
    }
}
