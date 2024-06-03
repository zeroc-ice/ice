//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import Ice
import TestCommon

func connect(_ prx: Ice.ObjectPrx) throws -> Ice.Connection {
    for _ in 0..<10 {
        do {
            _ = try prx.ice_getConnection()
            break
        } catch is Ice.ConnectTimeoutException {
            // Can sporadically occur with slow machines
        }
    }
    return try prx.ice_getConnection()!
}

public func allTests(helper: TestHelper) throws {
    let controller = try checkedCast(
        prx: helper.communicator().stringToProxy("controller:\(helper.getTestEndpoint(num: 1))")!,
        type: ControllerPrx.self
    )!
    do {
        try allTestsWithController(helper: helper, controller: controller)
    } catch {
        // Ensure the adapter is not in the holding state when an unexpected exception occurs to prevent
        // the test from hanging on exit in case a connection which disables timeouts is still opened.
        try controller.resumeAdapter()
        throw error
    }
}

public func allTestsWithController(helper: TestHelper, controller: ControllerPrx) throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    let communicator = helper.communicator()
    let sref = "timeout:\(helper.getTestEndpoint(num: 0))"
    let obj = try communicator.stringToProxy(sref)!

    let timeout = try checkedCast(prx: obj, type: TimeoutPrx.self)!

    let output = helper.getWriter()
    output.write("testing connect timeout... ")
    do {
        //
        // Expect ConnectTimeoutException.
        //
        let to = timeout.ice_connectionId("con1")
        try controller.holdAdapter(-1)
        do {
            try to.op()
            try test(false)
        } catch is Ice.ConnectTimeoutException {
            // Expected.
        }
        try controller.resumeAdapter()
        try timeout.op()  // Ensure adapter is active.
    }

    do {
        //
        // Expect success.
        //
        let to = timeout.ice_connectionId("con2")
        try controller.holdAdapter(100)
        do {
            try to.op()
        } catch is Ice.ConnectTimeoutException {
            try test(false)
        }
    }
    output.writeLine("ok")

    output.write("testing invocation timeout... ")
    do {
        let connection = try obj.ice_getConnection()
        var to = timeout.ice_invocationTimeout(100)
        try test(connection === to.ice_getConnection())
        do {
            try to.sleep(1000)
            try test(false)
        } catch is Ice.InvocationTimeoutException {}
        try obj.ice_ping()
        to = timeout.ice_invocationTimeout(1000)
        try test(connection === to.ice_getConnection())
        do {
            try to.sleep(100)
        } catch is Ice.InvocationTimeoutException {
            try test(false)
        }
        try test(connection === to.ice_getConnection())
    }

    do {
        //
        // Expect InvocationTimeoutException.
        //
        let to = timeout.ice_invocationTimeout(100)
        do {
            try to.sleepAsync(500).wait()
            try test(false)
        } catch is Ice.InvocationTimeoutException {}
        try timeout.ice_ping()
    }

    do {
        //
        // Expect success.
        //
        let to = timeout.ice_invocationTimeout(1000)
        do {
            try to.sleepAsync(100).wait()
        } catch {
            try test(false)
        }
    }
    output.writeLine("ok")

    output.write("testing close timeout... ")
    do {
        let to = timeout
        let connection = try connect(to)
        try controller.holdAdapter(-1)
        try connection.close(.GracefullyWithWait)
        do {
            _ = try connection.getInfo()  // getInfo() doesn't throw in the closing state.
        } catch is Ice.LocalException {
            try test(false)
        }

        while true {
            do {
                _ = try connection.getInfo()
                Thread.sleep(forTimeInterval: 0.01)
            } catch let ex as Ice.ConnectionManuallyClosedException {
                // Expected.
                try test(ex.graceful)
                break
            }
        }
        try controller.resumeAdapter()
        try timeout.op()  // Ensure adapter is active.
    }
    output.writeLine("ok")

    output.write("testing invocation timeouts with collocated calls... ")
    do {
        communicator.getProperties().setProperty(
            key: "TimeoutCollocated.AdapterId", value: "timeoutAdapter")

        let adapter = try communicator.createObjectAdapter("TimeoutCollocated")
        try adapter.activate()

        let proxy = try uncheckedCast(
            prx: adapter.addWithUUID(TimeoutDisp(TimeoutI())),
            type: TimeoutPrx.self
        ).ice_invocationTimeout(100)
        do {
            try proxy.sleep(500)
            try test(false)
        } catch is Ice.InvocationTimeoutException {}

        do {
            try proxy.sleepAsync(500).wait()
            try test(false)
        } catch is Ice.InvocationTimeoutException {}

        let batchTimeout = proxy.ice_batchOneway()
        try batchTimeout.ice_ping()
        try batchTimeout.ice_ping()
        try batchTimeout.ice_ping()

        _ = proxy.ice_invocationTimeout(-1).sleepAsync(500)  // Keep the server thread pool busy.
        do {
            try batchTimeout.ice_flushBatchRequestsAsync().wait()
            try test(false)
        } catch is Ice.InvocationTimeoutException {}
        adapter.destroy()
    }
    output.writeLine("ok")
    try controller.shutdown()
}
