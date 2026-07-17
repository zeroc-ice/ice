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
        communicator: communicator, proxyString: "test:\(helper.getTestEndpoint(num: 0))",
        type: TestIntfPrx.self)

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
            try await b1.ice_getConnection()!.close()

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
            try await b1.ice_getConnection()!.close()

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
            try await b1.ice_getConnection()!.close()
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
            try await b1.ice_getConnection()!.close()
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
            try await b1.ice_getConnection()!.close()
            try await b2.ice_getConnection()!.close()
            try await communicator.flushBatchRequests(.BasedOnProxy)
            try await test(p.opBatchCount() == 0)
        }
        output.writeLine("ok")
    }

    if try await p.ice_getConnection() != nil {
        output.write("testing connection close... ")
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
            try await con.close()
            try await r.value  // Should complete successfully.
            await cb.value
        }

        do {
            //
            // Remote case.
            //
            let seq = ByteSeq(repeating: 0, count: 1024 * 10)

            //
            // Send multiple opWithPayload, followed by a close and followed by multiple opWithPayload.
            // The goal is to make sure that none of the opWithPayload fail even if the server closes
            // the connection gracefully in between.
            //
            var maxQueue = 2
            while maxQueue < 50 {
                try await p.ice_ping()

                try await withThrowingTaskGroup(of: Void.self) { group in
                    for _ in 0..<maxQueue {
                        group.addTask {
                            try await p.opWithPayload(seq)
                        }
                    }

                    group.addTask {
                        try await p.closeConnection()
                    }

                    for _ in 0..<maxQueue {
                        group.addTask {
                            try await p.opWithPayload(seq)
                        }
                    }

                    try await group.waitForAll()

                    maxQueue *= 2
                }

                try await p.ice_getCachedConnection()!.close()
            }
        }

        output.writeLine("ok")

        if !collocated {
            output.write("testing bi-dir... ")
            let adapter = try communicator.createObjectAdapter("")
            let replyI = PingReplyI()
            let reply = try uncheckedCast(
                prx: adapter.addWithUUID(replyI), type: PingReplyPrx.self)

            let context: [String: String] = ["ONE": ""]
            try await p.pingBiDir(reply, context: context)

            try await p.ice_getConnection()!.setAdapter(adapter)
            try await p.pingBiDir(reply)
            try await test(replyI.checkReceived())
            adapter.destroy()
            output.writeLine("ok")
        }

        output.write("testing connection abort... ")
        do {
            //
            // Local case: start an operation and then close the connection forcefully on the client side.
            // There will be no retry and we expect the invocation to fail with ConnectionAbortedException.
            //
            try await p.ice_ping()
            let con = try await p.ice_getConnection()!

            async let startDispatch: Void = p.startDispatch()
            try await Task.sleep(for: .milliseconds(100))  // Wait for the request to be sent.
            con.abort()
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
                try await p.abortConnection()
                try test(false)
            } catch is Ice.ConnectionLostException {}  // Expected.
        }
        output.writeLine("ok")
    }
    try await p.shutdown()
}

actor PingReplyI: PingReply {
    private var _received = false

    func checkReceived() -> Bool {
        return _received
    }

    func reply(current: Ice.Current) throws {
        _received = true
    }
}
