//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import PromiseKit

func allTests(_ helper: TestHelper, collocated: Bool = false) throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    let communicator = helper.communicator()
    let output = helper.getWriter()

    var sref = "test:\(helper.getTestEndpoint(num: 0))"
    var obj = try communicator.stringToProxy(sref)!

    var p = uncheckedCast(prx: obj, type: TestIntfPrx.self)
    sref = "testController:\(helper.getTestEndpoint(num: 1))"
    obj = try communicator.stringToProxy(sref)!

    let testController = uncheckedCast(prx: obj, type: TestIntfControllerPrx.self)

    output.write("testing async invocation...")
    do {
        let ctx: [String: String] = [:]

        try test(p.ice_isAAsync(id: "::Test::TestIntf").wait())
        try test(p.ice_isAAsync(id: "::Test::TestIntf", context: ctx).wait())

        try p.ice_pingAsync().wait()
        try p.ice_pingAsync(context: ctx).wait()

        try test(p.ice_idAsync().wait() == "::Test::TestIntf")
        try test(p.ice_idAsync(context: ctx).wait() == "::Test::TestIntf")

        try test(p.ice_idsAsync().wait().count == 2)
        try test(p.ice_idsAsync(context: ctx).wait().count == 2)

        if !collocated {
            try test(p.ice_getConnectionAsync().wait() != nil)
        }

        try p.opAsync().wait()
        try p.opAsync(context: ctx).wait()

        try test(p.opWithResultAsync().wait() == 15)
        try test(p.opWithResultAsync(context: ctx).wait() == 15)

        do {
            try p.opWithUEAsync().wait()
            try test(false)
        } catch is TestIntfException {}

        do {
            try p.opWithUEAsync(context: ctx).wait()
            try test(false)
        } catch is TestIntfException {}
    }
    output.writeLine("ok")

    output.write("testing local exceptions... ")
    do {
        let indirect = uncheckedCast(prx: p.ice_adapterId("dummy"), type: TestIntfPrx.self)
        try indirect.opAsync().wait()
    } catch is Ice.NoEndpointException {}

    do {
        _ = try p.ice_oneway().opWithResultAsync().wait()
        try test(false)
    } catch is Ice.LocalException {}

    //
    // Check that CommunicatorDestroyedException is raised directly.
    //
    if try p.ice_getConnection() != nil {
        var initData = Ice.InitializationData()
        initData.properties = communicator.getProperties().clone()
        let ic = try helper.initialize(initData)
        let o = try ic.stringToProxy(p.ice_toString())!
        let p2 = try checkedCast(prx: o, type: TestIntfPrx.self)!
        ic.destroy()
        do {
            try p2.opAsync().wait()
            try test(false)
        } catch is Ice.CommunicatorDestroyedException {}
    }
    output.writeLine("ok")

    output.write("testing exception callback... ")
    do {
        let i = uncheckedCast(prx: p.ice_adapterId("dummy"), type: TestIntfPrx.self)

        do {
            _ = try i.ice_isAAsync(id: "::Test::TestIntf").wait()
            try test(false)
        } catch is Ice.NoEndpointException {}

        do {
            try i.opAsync().wait()
            try test(false)
        } catch is Ice.NoEndpointException {}

        do {
            _ = try i.opWithResultAsync().wait()
            try test(false)
        } catch is Ice.NoEndpointException {}

        do {
            try i.opWithUEAsync().wait()
            try test(false)
        } catch is Ice.NoEndpointException {}

        // Ensures no exception is called when response is received
        _ = try p.ice_isAAsync(id: "::Test::TestIntf").wait()
        try p.opAsync().wait()
        _ = try p.opWithResultAsync().wait()

        do {
            // If response is a user exception, it should be received.
            try p.opWithUEAsync().wait()
            try test(false)
        } catch is TestIntfException {}
    }
    output.writeLine("ok")

    output.write("testing sent callback... ")
    _ = try Promise<Bool> { seal in
        _ = p.ice_isAAsync(id: "", sent: { sentSynchronously in
                                       seal.fulfill(sentSynchronously)
                                   })
    }.wait()

    _ = try Promise<Bool> { seal in
        _ = p.ice_pingAsync(sent: { sentSynchronously in
                                seal.fulfill(sentSynchronously)
                            })
    }.wait()

    _ = try Promise<Bool> { seal in
        _ = p.ice_idAsync(sent: { sentSynchronously in
                              seal.fulfill(sentSynchronously)
                          })
    }.wait()

    _ = try Promise<Bool> { seal in
        _ = p.ice_idsAsync(sent: { sentSynchronously in
                               seal.fulfill(sentSynchronously)
                           })
    }.wait()

    _ = try Promise<Bool> { seal in
        _ = p.opAsync(sent: { sentSynchronously in
                          seal.fulfill(sentSynchronously)
                      })
    }.wait()

    let seq = [UInt8](repeating: 0, count: 1024)
    var cbs = [Promise<Bool>]()
    try testController.holdAdapter()
    var cb: Promise<Bool>!
    do {
        defer {
            do {
                try testController.resumeAdapter()
            } catch {}
        }

        while true {
            cb = Promise<Bool> { seal in
                _ = p.opWithPayloadAsync(seq,
                                         sent: { sentSynchronously in
                                             seal.fulfill(sentSynchronously)
                                         })
            }
            Thread.sleep(forTimeInterval: 0.01)
            cbs.append(cb)
            if try !cb.isFulfilled || !cb.wait() {
                break
            }
        }
    }
    try test(cb.wait() == false)

    for cb in cbs {
        _ = try cb.wait()
    }
    output.writeLine("ok")

    output.write("testing batch requests with proxy... ")
    do {
        try test(Promise<Bool> { seal in
                     _ = p.ice_batchOneway().ice_flushBatchRequestsAsync(sent: { sentSynchronously in
                                                                             seal.fulfill(sentSynchronously)
                                                                         })
                 }.wait())

        do {
            try test(p.opBatchCount() == 0)
            let b1 = p.ice_batchOneway()
            try b1.opBatch()
            let b1r = b1.opBatchAsync()
            try test(b1r.isFulfilled)
            var r: Promise<Void>!
            try Promise<Void> { seal in
                r = b1.ice_flushBatchRequestsAsync(sent: { _ in
                                                       seal.fulfill(())
                                                   })
            }.wait()
            try test(r.isResolved)
            try test(p.waitForBatch(2))
        }

        if try p.ice_getConnection() != nil {

            try test(p.opBatchCount() == 0)
            let b1 = p.ice_batchOneway()
            try b1.opBatch()
            try b1.ice_getConnection()!.close(.GracefullyWithWait)

            var r: Promise<Void>!
            try Promise<Void> { seal  in
                r = b1.ice_flushBatchRequestsAsync(sent: { _ in
                                                       seal.fulfill(())
                                                   })
            }.wait()
            try test(r.isResolved)

            try test(p.waitForBatch(1))
        }
    }
    output.writeLine("ok")

    if try p.ice_getConnection() != nil {
        output.write("testing batch requests with connection... ")
        do {
            do {
                try test(p.opBatchCount() == 0)
                let b1 = try uncheckedCast(prx: p.ice_getConnection()!.createProxy(p.ice_getIdentity()),
                                           type: TestIntfPrx.self).ice_batchOneway()
                try b1.opBatch()
                try b1.opBatch()
                var r: Promise<Void>!
                try Promise<Void> { seal in
                    r = try b1.ice_getConnection()!.flushBatchRequestsAsync(.BasedOnProxy,
                                                                            sent: { _ in
                                                                                seal.fulfill(())
                                                                            })
                }.wait()
                try r.wait()
                try test(r.isResolved)
                try test(p.waitForBatch(2))
            }

            try test(p.opBatchCount() == 0)
            let b1 = try uncheckedCast(prx: p.ice_getConnection()!.createProxy(p.ice_getIdentity()),
                                       type: TestIntfPrx.self).ice_batchOneway()
            try b1.opBatch()
            try b1.ice_getConnection()!.close(.GracefullyWithWait)

            let r = try b1.ice_getConnection()!.flushBatchRequestsAsync(.BasedOnProxy)
            try test(!r.isResolved)

            try test(p.waitForBatch(0))
        }
        output.writeLine("ok")

        output.write("testing batch requests with communicator... ")
        do {
            //
            // Async task - 1 connection.
            //
            try test(p.opBatchCount() == 0)
            let b1 = try uncheckedCast(prx: p.ice_getConnection()!.createProxy(p.ice_getIdentity()),
                                       type: TestIntfPrx.self).ice_batchOneway()
            try b1.opBatch()
            try b1.opBatch()
            var r: Promise<Void>!
            try Promise<Void> { seal in
                r = communicator.flushBatchRequestsAsync(.BasedOnProxy,
                                                         sent: { _ in seal.fulfill(()) })
            }.wait()
            try r.wait()
            try test(r.isResolved)
            try test(p.waitForBatch(2))
        }

        //
        // Async task exception - 1 connection.
        //
        do {
            try test(p.opBatchCount() == 0)
            let b1 = try uncheckedCast(prx: p.ice_getConnection()!.createProxy(p.ice_getIdentity()),
                                       type: TestIntfPrx.self).ice_batchOneway()
            try b1.opBatch()
            try b1.ice_getConnection()!.close(.GracefullyWithWait)
            var r: Promise<Void>!
            try Promise<Void> { seal in
                r = communicator.flushBatchRequestsAsync(.BasedOnProxy,
                                                         sent: { _ in
                                                             seal.fulfill(())
                                                         })
            }.wait()
            try test(r.isResolved)
            try test(p.opBatchCount() == 0)
        }

        //
        // Async task - 2 connections.
        //
        do {
            try test(p.opBatchCount() == 0)
            let b1 = try uncheckedCast(prx: p.ice_getConnection()!.createProxy(p.ice_getIdentity()),
                                       type: TestIntfPrx.self).ice_batchOneway()

            let con = try p.ice_connectionId("2").ice_getConnection()!
            let b2 = try uncheckedCast(prx: con.createProxy(p.ice_getIdentity()),
                                       type: TestIntfPrx.self).ice_batchOneway()

            _ = try b2.ice_getConnection() // Ensure connection is established.
            try b1.opBatch()
            try b1.opBatch()
            try b2.opBatch()
            try b2.opBatch()

            var r: Promise<Void>!
            try Promise<Void> { seal in
                r = communicator.flushBatchRequestsAsync(.BasedOnProxy,
                                                         sent: { _ in
                                                             seal.fulfill(())
                                                         })
            }.wait()
            try test(r.isResolved)
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
            let b1 = try uncheckedCast(prx: p.ice_getConnection()!.createProxy(p.ice_getIdentity()),
                                       type: TestIntfPrx.self).ice_batchOneway()

            let con = try p.ice_connectionId("2").ice_getConnection()!
            let b2 = try uncheckedCast(prx: con.createProxy(p.ice_getIdentity()),
                                       type: TestIntfPrx.self).ice_batchOneway()

            _ = try b2.ice_getConnection() // Ensure connection is established.
            try b1.opBatch()
            try b2.opBatch()
            try b1.ice_getConnection()!.close(.GracefullyWithWait)
            var r: Promise<Void>!
            try Promise<Void> { seal in
                r = communicator.flushBatchRequestsAsync(.BasedOnProxy,
                                                         sent: { _ in
                                                             seal.fulfill(())
                                                         })
            }.wait()
            try test(r.isResolved)
            try test(p.waitForBatch(1))
        }

        do {
            //
            // Async task exception - 2 connections - 2 failures.
            //
            // The sent callback should be invoked even if all connections fail.
            //
            try test(p.opBatchCount() == 0)
            let b1 = try uncheckedCast(prx: p.ice_getConnection()!.createProxy(p.ice_getIdentity()),
                                       type: TestIntfPrx.self).ice_batchOneway()

            let con = try p.ice_connectionId("2").ice_getConnection()!
            let b2 = try uncheckedCast(prx: con.createProxy(p.ice_getIdentity()),
                                       type: TestIntfPrx.self).ice_batchOneway()

            _ = try b2.ice_getConnection() // Ensure connection is established.
            try b1.opBatch()
            try b2.opBatch()
            try b1.ice_getConnection()!.close(.GracefullyWithWait)
            try b2.ice_getConnection()!.close(.GracefullyWithWait)
            var r: Promise<Void>!
            try Promise<Void> { seal in
                r = communicator.flushBatchRequestsAsync(.BasedOnProxy,
                                                         sent: { _ in
                                                             seal.fulfill(())
                                                         })
            }.wait()
            try test(r.isResolved)
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
            let cb = Promise<Void> { seal in
                do {
                    try con.setCloseCallback({ _ in seal.fulfill(()) })
                } catch {
                    precondition(false)
                }
            }
            let r = p.sleepAsync(100)
            try con.close(.GracefullyWithWait)
            try r.wait() // Should complete successfully.
            try cb.wait()
        }

        do {
            //
            // Remote case.
            //
            let seq = [UInt8](repeating: 0, count: 1024 * 10)

            //
            // Send multiple opWithPayload, followed by a close and followed by multiple opWithPaylod.
            // The goal is to make sure that none of the opWithPayload fail even if the server closes
            // the connection gracefully in between.
            //
            var maxQueue = 2
            var done = false
            while !done && maxQueue < 50 {
                done = true
                try p.ice_ping()
                var results: [Promise<Void>] = []
                for _ in 0..<maxQueue {
                    results.append(p.opWithPayloadAsync(seq))
                }

                var cb = Promise<Bool> { seal in
                    _ = p.closeAsync(.GracefullyWithWait, sent: {
                        seal.fulfill($0)
                    })
                }

                if try !cb.isResolved || cb.wait() {
                    for _ in 0..<maxQueue {

                        cb = Promise<Bool> { seal in
                            results.append(p.opWithPayloadAsync(seq, sent: { seal.fulfill($0) }))
                        }

                        if try cb.isResolved && cb.wait() {
                            done = false
                            maxQueue *= 2
                            break
                        }
                    }
                } else {
                    maxQueue *= 2
                    done = false
                }

                for p in results {
                    try p.wait()
                }
            }
        }
        output.writeLine("ok")

        output.write("testing graceful close connection without wait... ")
        do {
            //
            // Local case: start an operation and then close the connection gracefully on the client side
            // without waiting for the pending invocation to complete. There will be no retry and we expect the
            // invocation to fail with ConnectionManuallyClosedException.
            //
            p = p.ice_connectionId("CloseGracefully") // Start with a new connection.
            var con = try p.ice_getConnection()!

            var t: Promise<Void>!

            _ = try Promise<Bool> { seal in
                t = p.startDispatchAsync(sent: { seal.fulfill($0) })
            }.wait() // Ensure the request was sent before we close the connection.
            try con.close(.Gracefully)

            do {
                try t.wait()
                try test(false)
            } catch let ex as Ice.ConnectionManuallyClosedException {
                try test(ex.graceful)
            }
            try p.finishDispatch()

            //
            // Remote case: the server closes the connection gracefully, which means the connection
            // will not be closed until all pending dispatched requests have completed.
            //
            con = try p.ice_getConnection()!

            let cb = Promise<Void> { seal in
                try con.setCloseCallback({ _ in
                        seal.fulfill(())
                    })
            }
            t = p.sleepAsync(100)
            try p.close(.Gracefully) // Close is delayed until sleep completes.
            try cb.wait()
            try t.wait()
        }
        output.writeLine("ok")

        output.write("testing forceful close connection... ")
        do {
            //
            // Local case: start an operation and then close the connection forcefully on the client side.
            // There will be no retry and we expect the invocation to fail with ConnectionManuallyClosedException.
            //
            try p.ice_ping()
            let con = try p.ice_getConnection()!
            var t: Promise<Void>!
            _ = try Promise<Bool> { seal in
                t = p.startDispatchAsync(sent: { seal.fulfill($0)})
            }.wait() // Ensure the request was sent before we close the connection.
            try con.close(.Forcefully)
            do {
                try t.wait()
                try test(false)
            } catch let ex as Ice.ConnectionManuallyClosedException {
                try test(!ex.graceful)
            }
            try p.finishDispatch()

            //
            // Remote case: the server closes the connection forcefully. This causes the request to fail
            // with a ConnectionLostException. Since the close() operation is not idempotent, the client
            // will not retry.
            //
            do {
                try p.close(.Forcefully)
                try test(false)
            } catch is Ice.ConnectionLostException {} // Expected.
        }
        output.writeLine("ok")
    }
    try p.shutdown()
}
