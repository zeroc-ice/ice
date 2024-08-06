// Copyright (c) ZeroC, Inc.

import Foundation
import Ice
import TestCommon

// WORKAROUND: Disable optimization for this test. Otherwise we get a crash in
// "testing batch requests with communicator"
@_optimize(none)
func allTests(_ helper: TestHelper, collocated: Bool = false) async throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    let communicator = helper.communicator()
    let output = helper.getWriter()

    let p = try makeProxy(
        communicator: communicator, proxyString: "test:\(helper.getTestEndpoint(num: 0))", type: TestIntfPrx.self)

    output.write("testing async invocation...")
    do {
        let ctx: [String: String] = [:]

        try await test(p.ice_isA(id: "::Test::TestIntf"))
        try await test(p.ice_isA(id: "::Test::TestIntf", context: ctx))

        try await p.ice_ping()
        try await p.ice_ping(context: ctx)

        try await test(p.ice_id() == "::Test::TestIntf")
        try await test(p.ice_id(context: ctx) == "::Test::TestIntf")

        try await test(p.ice_ids().count == 2)
        try await test(p.ice_ids(context: ctx).count == 2)

        if !collocated {
            try await test(p.ice_getConnection() != nil)
        }

        try await p.op()
        try await p.op(context: ctx)

        try await test(p.opWithResult() == 15)
        try await test(p.opWithResult(context: ctx) == 15)

        do {
            try await p.opWithUE()
            try test(false)
        } catch is TestIntfException {}

        do {
            try await p.opWithUE(context: ctx)
            try test(false)
        } catch is TestIntfException {}
    }
    output.writeLine("ok")

    output.write("testing local exceptions... ")
    do {
        let indirect = p.ice_adapterId("dummy")
        try await indirect.op()
    } catch is Ice.NoEndpointException {}

    do {
        _ = try await p.ice_oneway().opWithResult()
        try test(false)
    } catch is Ice.LocalException {}

    // Check that CommunicatorDestroyedException is raised directly.
    if try await p.ice_getConnection() != nil {
        var initData = Ice.InitializationData()
        initData.properties = communicator.getProperties().clone()
        let ic = try helper.initialize(initData)
        let p2 = try makeProxy(communicator: ic, proxyString: p.ice_toString(), type: TestIntfPrx.self)
        try await p2.ice_ping()
        ic.destroy()
        do {
            try await p2.op()
            try test(false)
        } catch is Ice.CommunicatorDestroyedException {}
    }
    output.writeLine("ok")

    output.write("testing exceptions with async/await... ")
    do {
        let i = p.ice_adapterId("dummy")

        do {
            _ = try await i.ice_isA(id: "::Test::TestIntf")
            try test(false)
        } catch is Ice.NoEndpointException {}

        do {
            try await i.op()
            try test(false)
        } catch is Ice.NoEndpointException {}

        do {
            _ = try await i.opWithResult()
            try test(false)
        } catch is Ice.NoEndpointException {}

        do {
            try await i.opWithUE()
            try test(false)
        } catch is Ice.NoEndpointException {}

        // Ensure no exception is thrown when response is received
        _ = try await p.ice_isA(id: "::Test::TestIntf")
        try await p.op()
        _ = try await p.opWithResult()

        do {
            // If response is a user exception, it should be received.
            try await p.opWithUE()
            try test(false)
        } catch is TestIntfException {}
    }
    output.writeLine("ok")

    output.write("testing batch requests with proxy... ")
    do {
        do {
            try await test(p.opBatchCount() == 0)
            let b1 = p.ice_batchOneway()
            try await b1.opBatch()
            try await b1.opBatch()
            try await b1.ice_flushBatchRequests()
            try await test(p.waitForBatch(2))
        }

        if try await p.ice_getConnection() != nil {
            try await test(p.opBatchCount() == 0)
            let b1 = p.ice_batchOneway()
            try await b1.opBatch()
            try await b1.ice_getConnection()!.close(.GracefullyWithWait)

            try await b1.ice_flushBatchRequests()
            try await test(p.waitForBatch(1))
        }
    }
    output.writeLine("ok")

    if try await p.ice_getConnection() != nil {
        output.write("testing batch requests with connection... ")
        do {
            do {
                try await test(p.opBatchCount() == 0)
                let b1 = try await p.ice_fixed(p.ice_getConnection()!).ice_batchOneway()
                try await b1.opBatch()
                try await b1.opBatch()

                try await b1.ice_getConnection()!.flushBatchRequests(.BasedOnProxy)
                try await test(p.waitForBatch(2))
            }

            try await test(p.opBatchCount() == 0)
            let b1 = try await p.ice_fixed(p.ice_getConnection()!).ice_batchOneway()
            try await b1.opBatch()
            try await b1.ice_getConnection()!.close(.GracefullyWithWait)

            do {
                try await b1.ice_getConnection()!.flushBatchRequests(.BasedOnProxy)
                try test(false)
            } catch is Ice.LocalException {}
            try await test(p.waitForBatch(0))
            try await test(p.opBatchCount() == 0)
        }
        output.writeLine("ok")

        output.write("testing batch requests with communicator... ")
        do {
            //
            // Async task - 1 connection.
            //
            try await test(p.opBatchCount() == 0)
            let b1 = try await p.ice_fixed(p.ice_getConnection()!).ice_batchOneway()
            try await b1.opBatch()
            try await b1.opBatch()
            try await communicator.flushBatchRequests(.BasedOnProxy)
            try await test(p.waitForBatch(2))
        }

        //
        // Async task exception - 1 connection.
        //
        do {
            try await test(p.opBatchCount() == 0)
            let b1 = try await p.ice_fixed(p.ice_getConnection()!).ice_batchOneway()
            try await b1.opBatch()
            try await b1.ice_getConnection()!.close(.GracefullyWithWait)
            try await communicator.flushBatchRequests(.BasedOnProxy)
            try await test(p.opBatchCount() == 0)
        }

        //
        // Async task - 2 connections.
        //
        do {
            try await test(p.opBatchCount() == 0)
            let b1 = try await p.ice_fixed(p.ice_getConnection()!).ice_batchOneway()
            let con = try await p.ice_connectionId("2").ice_getConnection()!
            let b2 = p.ice_fixed(con).ice_batchOneway()
            try await b1.opBatch()
            try await b1.opBatch()
            try await b2.opBatch()
            try await b2.opBatch()
            try await communicator.flushBatchRequests(.BasedOnProxy)
            try await test(p.waitForBatch(4))
        }

        do {
            //
            // AsyncResult exception - 2 connections - 1 failure.
            //
            // All connections should be flushed even if there are failures on some connections.
            // Exceptions should not be reported.
            //
            try await test(p.opBatchCount() == 0)
            let b1 = try await p.ice_fixed(p.ice_getConnection()!).ice_batchOneway()

            let con = try await p.ice_connectionId("2").ice_getConnection()!
            let b2 = p.ice_fixed(con).ice_batchOneway()
            try await b1.opBatch()
            try await b2.opBatch()
            try await b1.ice_getConnection()!.close(.GracefullyWithWait)
            try await communicator.flushBatchRequests(.BasedOnProxy)
            try await test(p.waitForBatch(1))
        }

        do {
            //
            // Async task exception - 2 connections - 2 failures.
            //
            // The sent callback should be invoked even if all connections fail.
            //
            try await test(p.opBatchCount() == 0)
            let b1 = try await p.ice_fixed(p.ice_getConnection()!).ice_batchOneway()

            let con = try await p.ice_connectionId("2").ice_getConnection()!
            let b2 = p.ice_fixed(con).ice_batchOneway()
            try await b1.opBatch()
            try await b2.opBatch()
            try await b1.ice_getConnection()!.close(.GracefullyWithWait)
            try await b2.ice_getConnection()!.close(.GracefullyWithWait)
            try await communicator.flushBatchRequests(.BasedOnProxy)
            try await test(p.opBatchCount() == 0)
        }
        output.writeLine("ok")
    }

    if try await p.ice_getConnection() != nil, try await p.supportsAMD() {
        output.write("testing graceful close connection with wait... ")
        do {
            //
            // Local case: begin a request, close the connection gracefully, and make sure it waits
            // for the request to complete.
            //
            let con = try await p.ice_getConnection()!

            async let cb = Task {
                await withCheckedContinuation { continuation in
                    do {
                        try con.setCloseCallback { _ in continuation.resume() }
                    } catch {
                        fatalError("unexpected error: \(error)")
                    }
                }
            }

            let p1 = p
            async let r = Task { try await p1.sleep(100) }
            try con.close(.GracefullyWithWait)
            try await r.value  // Should complete successfully.
            await cb.value
        }

        // TODO: Update to use async/await
        //     do {
        //         //
        //         // Remote case.
        //         //
        //         let seq = ByteSeq(repeating: 0, count: 1024 * 10)

        //         //
        //         // Send multiple opWithPayload, followed by a close and followed by multiple opWithPaylod.
        //         // The goal is to make sure that none of the opWithPayload fail even if the server closes
        //         // the connection gracefully in between.
        //         //
        //         var maxQueue = 2
        //         var done = false
        //         while !done, maxQueue < 50 {
        //             done = true
        //             try p.ice_ping()
        //             var results: [Promise<Void>] = []
        //             for _ in 0..<maxQueue {
        //                 results.append(p.opWithPayload(seq))
        //             }

        //             var cb = Promise<Bool> { seal in
        //                 _ = p.close(.GracefullyWithWait) {
        //                     seal.fulfill($0)
        //                 }
        //             }

        //             if try !cb.isResolved || cb.wait() {
        //                 for _ in 0..<maxQueue {
        //                     cb = Promise<Bool> { seal in
        //                         results.append(p.opWithPayload(seq) { seal.fulfill($0) })
        //                     }

        //                     if try cb.isResolved && cb.wait() {
        //                         done = false
        //                         maxQueue *= 2
        //                         break
        //                     }
        //                 }
        //             } else {
        //                 maxQueue *= 2
        //                 done = false
        //             }

        //             for p in results {
        //                 try p.wait()
        //             }
        //         }
        //     }
        output.writeLine("ok")

        output.write("testing graceful close connection without wait... ")
        do {
            //
            // Local case: start an operation and then close the connection gracefully on the client side
            // without waiting for the pending invocation to complete. There will be no retry and we expect the
            // invocation to fail with ConnectionClosedException.
            //
            let p = p.ice_connectionId("CloseGracefully")  // Start with a new connection.
            let con = try await p.ice_getConnection()!

            do {
                // Ensure the request was sent before we close the connection. Oneway invocations are
                // completed when the request is sent.
                async let startDispatch: Void = p.startDispatch()
                try await Task.sleep(for: .milliseconds(100))  // Wait for the request to be sent.
                try con.close(.Gracefully)
                try await startDispatch
                try test(false)

            } catch let ex as Ice.ConnectionClosedException {
                try test(ex.closedByApplication)
            }
            try await p.finishDispatch()
        }

        do {
            //
            // Remote case: the server closes the connection gracefully, which means the connection
            // will not be closed until all pending dispatched requests have completed.
            //
            let con = try await p.ice_getConnection()!

            async let cb = Task {
                await withCheckedContinuation { continuation in
                    do {
                        try con.setCloseCallback { _ in continuation.resume() }
                    } catch {
                        fatalError("unexpected error: \(error)")
                    }
                }
            }

            async let t = Task { try await p.sleep(100) }
            try await p.close(.Gracefully)  // Close is delayed until sleep completes.
            await cb.value
            try await t.value
        }
        output.writeLine("ok")

        output.write("testing forceful close connection... ")
        do {
            //
            // Local case: start an operation and then close the connection forcefully on the client side.
            // There will be no retry and we expect the invocation to fail with ConnectionAbortedException.
            //
            try await p.ice_ping()
            let con = try await p.ice_getConnection()!

            async let startDispatch: Void = p.startDispatch()
            try await Task.sleep(for: .milliseconds(100))  // Wait for the request to be sent.
            try con.close(.Forcefully)
            do {
                try await startDispatch
                try test(false)
            } catch let ex as Ice.ConnectionAbortedException {
                try test(ex.closedByApplication)
            }
            try await p.finishDispatch()

            //
            // Remote case: the server closes the connection forcefully. This causes the request to fail
            // with a ConnectionLostException. Since the close() operation is not idempotent, the client
            // will not retry.
            //
            do {
                try await p.close(.Forcefully)
                try test(false)
            } catch is Ice.ConnectionLostException {}  // Expected.
        }
        output.writeLine("ok")
    }
    try await p.shutdown()
}
