// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test } from "./Test.js";
import { TestHelper } from "../../Common/TestHelper.js";

const test = TestHelper.test;

export async function oneways(prx: Test.MyClassPrx) {
    prx = prx.ice_oneway();
    await prx.ice_ping();

    try {
        await prx.ice_isA(Test.MyClass.ice_staticId());
        test(false);
    } catch (ex) {
        // Expected: twoway proxy required
        test(ex instanceof Ice.TwowayOnlyException, ex);
    }

    try {
        await prx.ice_id();
        test(false);
    } catch (ex) {
        // Expected: twoway proxy required
        test(ex instanceof Ice.TwowayOnlyException, ex);
    }

    try {
        await prx.ice_ids();
        test(false);
    } catch (ex) {
        // Expected: twoway proxy required
        test(ex instanceof Ice.TwowayOnlyException, ex);
    }

    await prx.opVoid();
    await prx.opIdempotent();

    try {
        await prx.opByte(0xff, 0x0f);
        test(false);
    } catch (ex) {
        // Expected: twoway proxy required
        test(ex instanceof Ice.TwowayOnlyException, ex);
    }
}
