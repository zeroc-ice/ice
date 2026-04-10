// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test } from "./Test.js";
import { test } from "../../Common/TestHelper.js";

export async function oneways(prx: Test.MyClassPrx) {
    prx = prx.ice_oneway();
    await prx.ice_ping();

    try {
        await prx.ice_isA(Test.MyClass.ice_staticId());
        test(false);
    } catch (ex) {
        // Expected: twoway proxy required
        test(ex instanceof Ice.TwowayOnlyException, ex as Error);
    }

    try {
        await prx.ice_id();
        test(false);
    } catch (ex) {
        // Expected: twoway proxy required
        test(ex instanceof Ice.TwowayOnlyException, ex as Error);
    }

    try {
        await prx.ice_ids();
        test(false);
    } catch (ex) {
        // Expected: twoway proxy required
        test(ex instanceof Ice.TwowayOnlyException, ex as Error);
    }

    await prx.opVoid();
    await prx.opIdempotent();

    // Calling a ["oneway"] operation on a oneway proxy succeeds.
    await prx.opOneway();

    // Calling a ["oneway"] operation on a twoway proxy throws OnewayOnlyException.
    try {
        await prx.ice_twoway().opOneway();
        test(false);
    } catch (ex) {
        test(ex instanceof Ice.OnewayOnlyException, ex as Error);
    }

    try {
        await prx.opByte(0xff, 0x0f);
        test(false);
    } catch (ex) {
        // Expected: twoway proxy required
        test(ex instanceof Ice.TwowayOnlyException, ex as Error);
    }
}
