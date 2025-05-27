// Copyright (c) ZeroC, Inc.

import Foundation
import Ice
import TestCommon

func connect(_ prx: Ice.ObjectPrx) async throws -> Ice.Connection {
    for _ in 0..<10 {
        do {
            _ = try await prx.ice_getConnection()
            break
        } catch is Ice.ConnectTimeoutException {
            // Can sporadically occur with slow machines
        }
    }
    return try await prx.ice_getConnection()!
}

public func allTests(helper: TestHelper) async throws {
    let controller = try makeProxy(
        communicator: helper.communicator(),
        proxyString: "controller:\(helper.getTestEndpoint(num: 1))",
        type: ControllerPrx.self)

    _ = try await connect(controller)

    do {
        try await allTestsWithController(helper: helper, controller: controller)
    } catch {
        // Ensure the adapter is not in the holding state when an unexpected exception occurs to prevent
        // the test from hanging on exit in case a connection which disables timeouts is still opened.
        try await controller.resumeAdapter()
        throw error
    }
}

public func allTestsWithController(helper: TestHelper, controller: ControllerPrx) async throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    let communicator = helper.communicator()
    let sref = "timeout:\(helper.getTestEndpoint(num: 0))"
    let obj = try communicator.stringToProxy(sref)!

    let timeout = try await checkedCast(prx: obj, type: TimeoutPrx.self)!

    let output = helper.getWriter()
    output.write("testing connect timeout... ")
    do {
        //
        // Expect ConnectTimeoutException.
        //
        let to = timeout.ice_connectionId("con1")
        try await controller.holdAdapter(-1)
        do {
            try await to.op()
            try test(false)
        } catch is Ice.ConnectTimeoutException {
            // Expected.
        }
        try await controller.resumeAdapter()
        try await timeout.op()  // Ensure adapter is active.
    }

    do {
        //
        // Expect success.
        //
        let to = timeout.ice_connectionId("con2")
        try await controller.holdAdapter(100)
        do {
            try await to.op()
        } catch is Ice.ConnectTimeoutException {
            try test(false)
        }
    }
    output.writeLine("ok")

    output.write("testing invocation timeout... ")
    do {
        let connection = try await obj.ice_getConnection()
        var to = timeout.ice_invocationTimeout(100)
        try await test(connection === to.ice_getConnection())
        do {
            try await to.sleep(1000)
            try test(false)
        } catch is Ice.InvocationTimeoutException {}
        try await obj.ice_ping()
        to = timeout.ice_invocationTimeout(1000)
        try await test(connection === to.ice_getConnection())
        do {
            try await to.sleep(100)
        } catch is Ice.InvocationTimeoutException {
            try test(false)
        }
        try await test(connection === to.ice_getConnection())
    }

    do {
        //
        // Expect InvocationTimeoutException.
        //
        let to = timeout.ice_invocationTimeout(100)
        do {
            try await to.sleep(500)
            try test(false)
        } catch is Ice.InvocationTimeoutException {}
        try await timeout.ice_ping()
    }

    do {
        //
        // Expect success.
        //
        let to = timeout.ice_invocationTimeout(1000)
        do {
            try await to.sleep(100)
        } catch {
            try test(false)
        }
    }
    output.writeLine("ok")

    output.write("testing close timeout... ")
    do {
        let to = timeout
        let connection = try await connect(to)
        try await controller.holdAdapter(-1)
        async let closed: () = connection.close()
        do {
            _ = try connection.getInfo()  // getInfo() doesn't throw in the closing state.
        } catch is Ice.LocalException {
            try test(false)
        }

        do {
            try await closed  // wait for the closure to complete
            try test(false)
        } catch is Ice.CloseTimeoutException {
            // Expected.
        }

        do {
            _ = try connection.getInfo()
            try test(false)
        } catch is Ice.CloseTimeoutException {
            // Expected.
        }

        try await controller.resumeAdapter()
        try await timeout.op()  // Ensure adapter is active.
    }
    output.writeLine("ok")

    output.write("testing invocation timeouts with collocated calls... ")
    do {
        communicator.getProperties().setProperty(
            key: "TimeoutCollocated.AdapterId", value: "timeoutAdapter")

        let adapter = try communicator.createObjectAdapter("TimeoutCollocated")
        try adapter.activate()

        let proxy = try uncheckedCast(
            prx: adapter.addWithUUID(TimeoutI()),
            type: TimeoutPrx.self
        ).ice_invocationTimeout(100)
        do {
            try await proxy.sleep(500)
            try test(false)
        } catch is Ice.InvocationTimeoutException {}

        do {
            try await proxy.sleep(500)
            try test(false)
        } catch is Ice.InvocationTimeoutException {}

        adapter.destroy()
    }
    output.writeLine("ok")
    try await controller.shutdown()
}
