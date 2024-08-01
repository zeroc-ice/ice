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

        try await test(p.ice_isAAsync(id: "::Test::TestIntf"))
        try await test(p.ice_isAAsync(id: "::Test::TestIntf", context: ctx))

        try await p.ice_pingAsync()
        try await p.ice_pingAsync(context: ctx)

        try await test(p.ice_idAsync() == "::Test::TestIntf")
        try await test(p.ice_idAsync(context: ctx) == "::Test::TestIntf")

        try await test(p.ice_idsAsync().count == 2)
        try await test(p.ice_idsAsync(context: ctx).count == 2)

        if !collocated {
            try await test(p.ice_getConnectionAsync() != nil)
        }

        try await p.opAsync()
        try await p.opAsync(context: ctx)

        try await test(p.opWithResultAsync() == 15)
        try await test(p.opWithResultAsync(context: ctx) == 15)

        do {
            try await p.opWithUEAsync()
            try test(false)
        } catch is TestIntfException {}

        do {
            try await p.opWithUEAsync(context: ctx)
            try test(false)
        } catch is TestIntfException {}
    }
    output.writeLine("ok")

    output.write("testing local exceptions... ")
    do {
        let indirect = p.ice_adapterId("dummy")
        try await indirect.opAsync()
    } catch is Ice.NoEndpointException {}

    do {
        _ = try await p.ice_oneway().opWithResultAsync()
        try test(false)
    } catch is Ice.LocalException {}

    // Check that CommunicatorDestroyedException is raised directly.
    if try p.ice_getConnection() != nil {
        var initData = Ice.InitializationData()
        initData.properties = communicator.getProperties().clone()
        let ic = try helper.initialize(initData)
        let p2 = try makeProxy(communicator: ic, proxyString: p.ice_toString(), type: TestIntfPrx.self)
        try await p2.ice_pingAsync()
        ic.destroy()
        do {
            try await p2.opAsync()
            try test(false)
        } catch is Ice.CommunicatorDestroyedException {}
    }
    output.writeLine("ok")

    output.write("testing exceptions with async/await... ")
    do {
        let i = p.ice_adapterId("dummy")

        do {
            _ = try await i.ice_isAAsync(id: "::Test::TestIntf")
            try test(false)
        } catch is Ice.NoEndpointException {}

        do {
            try await i.opAsync()
            try test(false)
        } catch is Ice.NoEndpointException {}

        do {
            _ = try await i.opWithResultAsync()
            try test(false)
        } catch is Ice.NoEndpointException {}

        do {
            try await i.opWithUEAsync()
            try test(false)
        } catch is Ice.NoEndpointException {}

        // Ensure no exception is thrown when response is received
        _ = try await p.ice_isAAsync(id: "::Test::TestIntf")
        try await p.opAsync()
        _ = try await p.opWithResultAsync()

        do {
            // If response is a user exception, it should be received.
            try await p.opWithUEAsync()
            try test(false)
        } catch is TestIntfException {}
    }
    output.writeLine("ok")

    output.write("testing batch requests with proxy... ")
    do {
        do {
            try test(p.opBatchCount() == 0)
            let b1 = p.ice_batchOneway()
            try b1.opBatch()
            try await b1.opBatchAsync()
            try await b1.ice_flushBatchRequestsAsync()
            try test(p.waitForBatch(2))
        }

        if try p.ice_getConnection() != nil {
            try test(p.opBatchCount() == 0)
            let b1 = p.ice_batchOneway()
            try b1.opBatch()
            try b1.ice_getConnection()!.close(.GracefullyWithWait)

            try await b1.ice_flushBatchRequestsAsync()
            try test(p.waitForBatch(1))
        }
    }
    output.writeLine("ok")

    if try p.ice_getConnection() != nil {
        output.write("testing batch requests with connection... ")
        do {
            do {
                try test(p.opBatchCount() == 0)
                let b1 = try p.ice_fixed(p.ice_getConnection()!).ice_batchOneway()
                try b1.opBatch()
                try b1.opBatch()

                try await b1.ice_getConnection()!.flushBatchRequestsAsync(.BasedOnProxy)
                try test(p.waitForBatch(2))
            }

            try test(p.opBatchCount() == 0)
            let b1 = try p.ice_fixed(p.ice_getConnection()!).ice_batchOneway()
            try b1.opBatch()
            try b1.ice_getConnection()!.close(.GracefullyWithWait)

            do {
                try await b1.ice_getConnection()!.flushBatchRequestsAsync(.BasedOnProxy)
                try test(false)
            } catch is Ice.LocalException {}
            try test(p.waitForBatch(0))
            try test(p.opBatchCount() == 0)
        }
        output.writeLine("ok")

        output.write("testing batch requests with communicator... ")
        do {
            //
            // Async task - 1 connection.
            //
            try test(p.opBatchCount() == 0)
            let b1 = try p.ice_fixed(p.ice_getConnection()!).ice_batchOneway()
            try b1.opBatch()
            try b1.opBatch()
            try await communicator.flushBatchRequestsAsync(.BasedOnProxy)
            try test(p.waitForBatch(2))
        }

        //
        // Async task exception - 1 connection.
        //
        do {
            try test(p.opBatchCount() == 0)
            let b1 = try p.ice_fixed(p.ice_getConnection()!).ice_batchOneway()
            try b1.opBatch()
            try b1.ice_getConnection()!.close(.GracefullyWithWait)
            try await communicator.flushBatchRequestsAsync(.BasedOnProxy)
            try test(p.opBatchCount() == 0)
        }

        //
        // Async task - 2 connections.
        //
        do {
            try test(p.opBatchCount() == 0)
            let b1 = try p.ice_fixed(p.ice_getConnection()!).ice_batchOneway()
            let con = try p.ice_connectionId("2").ice_getConnection()!
            let b2 = p.ice_fixed(con).ice_batchOneway()
            try b1.opBatch()
            try b1.opBatch()
            try b2.opBatch()
            try b2.opBatch()
            try await communicator.flushBatchRequestsAsync(.BasedOnProxy)
            try test(p.waitForBatch(4))
        }

        do {
            //
            // AsyncResult exception - 2 connections - 1 failure.
            //
            // All connections should be flushed even if there are failures on some connections.
            // Exceptions should not be reported.
            //
            try test(p.opBatchCount() == 0)
            let b1 = try p.ice_fixed(p.ice_getConnection()!).ice_batchOneway()

            let con = try p.ice_connectionId("2").ice_getConnection()!
            let b2 = p.ice_fixed(con).ice_batchOneway()
            try b1.opBatch()
            try b2.opBatch()
            try b1.ice_getConnection()!.close(.GracefullyWithWait)
            try await communicator.flushBatchRequestsAsync(.BasedOnProxy)
            try test(p.waitForBatch(1))
        }

        do {
            //
            // Async task exception - 2 connections - 2 failures.
            //
            // The sent callback should be invoked even if all connections fail.
            //
            try test(p.opBatchCount() == 0)
            let b1 = try p.ice_fixed(p.ice_getConnection()!).ice_batchOneway()

            let con = try p.ice_connectionId("2").ice_getConnection()!
            let b2 = p.ice_fixed(con).ice_batchOneway()
            try b1.opBatch()
            try b2.opBatch()
            try b1.ice_getConnection()!.close(.GracefullyWithWait)
            try b2.ice_getConnection()!.close(.GracefullyWithWait)
            try await communicator.flushBatchRequestsAsync(.BasedOnProxy)
            try test(p.opBatchCount() == 0)
        }
        output.writeLine("ok")
    }

    if try p.ice_getConnection() != nil && p.supportsAMD() {
        output.write("testing graceful close connection with wait... ")
        do {
            //
            // Local case: begin a request, close the connection gracefully, and make sure it waits
            // for the request to complete.
            //
            let con = try p.ice_getConnection()!

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
            async let r = Task { try await p1.sleepAsync(100) }
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
        //                 results.append(p.opWithPayloadAsync(seq))
        //             }

        //             var cb = Promise<Bool> { seal in
        //                 _ = p.closeAsync(.GracefullyWithWait) {
        //                     seal.fulfill($0)
        //                 }
        //             }

        //             if try !cb.isResolved || cb.wait() {
        //                 for _ in 0..<maxQueue {
        //                     cb = Promise<Bool> { seal in
        //                         results.append(p.opWithPayloadAsync(seq) { seal.fulfill($0) })
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
            let con = try p.ice_getConnection()!

            do {
                // Ensure the request was sent before we close the connection. Oneway invocations are
                // completed when the request is sent.
                try await p.ice_oneway().startDispatchAsync()
                try! con.close(.Gracefully)

            } catch let ex as Ice.ConnectionClosedException {
                try test(ex.closedByApplication)
            }
            try p.finishDispatch()
        }
        do {
            //
            // Remote case: the server closes the connection gracefully, which means the connection
            // will not be closed until all pending dispatched requests have completed.
            //
            let con = try p.ice_getConnection()!

            async let cb = Task {
                await withCheckedContinuation { continuation in
                    do {
                        try con.setCloseCallback { _ in continuation.resume() }
                    } catch {
                        fatalError("unexpected error: \(error)")
                    }
                }
            }

            async let t = Task { try await p.sleepAsync(100) }
            try p.close(.Gracefully)  // Close is delayed until sleep completes.
            await cb.value
            try await t.value
        }
        output.writeLine("ok")
    }
    try p.shutdown()
}
