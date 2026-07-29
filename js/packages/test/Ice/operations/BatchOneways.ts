// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test } from "./Test.js";
import { TestHelper, test } from "../../Common/TestHelper.js";

export async function batchOneways(prx: Test.MyInterfacePrx, helper: TestHelper) {
    const bs1 = new Uint8Array(10 * 1024);
    for (let i = 0; i < bs1.length; ++i) {
        bs1[i] = 0;
    }

    const batch = Test.MyInterfacePrx.uncheckedCast(prx.ice_batchOneway());
    await batch.ice_flushBatchRequests();

    let r = batch.ice_flushBatchRequests();
    await r;
    test(r.isCompleted()); // Empty flush

    r = batch.ice_flushBatchRequests();
    await r;
    test(r.isSent()); // Empty flush

    r = batch.ice_flushBatchRequests();
    await r;
    test(r.isCompleted()); // Empty flush

    for (let i = 0; i < 30; ++i) {
        await batch.opByteSOneway(bs1);
    }

    let count = 0;
    while (count < 27) {
        // 3 * 9 requests auto-flushed.
        count += await prx.opByteSOnewayCallCount();
        await Ice.Promise.delay(10);
    }

    if (batch.ice_getConnection() !== null) {
        const batch1 = Test.MyInterfacePrx.uncheckedCast(prx.ice_batchOneway());
        const batch2 = Test.MyInterfacePrx.uncheckedCast(prx.ice_batchOneway());

        batch1.ice_ping();
        batch2.ice_ping();
        await batch1.ice_flushBatchRequests();
        await batch1.ice_getConnection().then((c) => c.close());
        batch1.ice_ping();
        batch2.ice_ping();

        await batch1.ice_getConnection();
        await batch2.ice_getConnection();

        batch1.ice_ping();
        await batch1.ice_getConnection().then((c) => c.close());

        batch1.ice_ping();
        batch2.ice_ping();
    }

    const identity = new Ice.Identity();
    identity.name = "invalid";
    const batch3 = batch.ice_identity(identity);
    batch3.ice_ping();
    await batch3.ice_flushBatchRequests();

    // Make sure that a bogus batch request doesn't cause troubles to other ones.
    batch3.ice_ping();
    batch.ice_ping();
    await batch.ice_flushBatchRequests();
    await batch.ice_ping();

    // Nothing listens on this endpoint.
    const unreachable = Test.MyInterfacePrx.uncheckedCast(
        new Test.MyInterfacePrx(prx.ice_getCommunicator(), `test:${helper.getTestEndpoint(1)}`).ice_batchOneway(),
    );

    // Filling the batch past Ice.BatchAutoFlushSize triggers an auto-flush that cannot connect. It fails silently:
    // the batched requests are lost, like any other oneway, and the failure is not reported to the caller that
    // happened to fill the batch.
    for (let i = 0; i < 11; ++i) {
        await unreachable.opByteSOneway(bs1);
    }
    try {
        // This flush fails the same way; by the time it does, the auto-flush above has settled too.
        await unreachable.ice_flushBatchRequests();
        test(false);
    } catch (ex) {
        test(ex instanceof Ice.LocalException, ex as Error);
    }
}
