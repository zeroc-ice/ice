//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import Ice
import TestCommon

func connect(_ prx: Ice.ObjectPrx) throws -> Ice.Connection {
    for _ in 0 ..< 10 {
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
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    let communicator = helper.communicator()
    let sref = "timeout:\(helper.getTestEndpoint(num: 0))"
    let obj = try communicator.stringToProxy(sref)!

    let timeout = try checkedCast(prx: obj, type: TimeoutPrx.self)!

    let controller = try checkedCast(prx: communicator.stringToProxy("controller:\(helper.getTestEndpoint(num: 1))")!,
                                     type: ControllerPrx.self)!

    let output = helper.getWriter()
    output.write("testing connect timeout... ")
    do {
        //
        // Expect ConnectTimeoutException.
        //
        let to = timeout.ice_timeout(100)
        try controller.holdAdapter(-1)
        do {
            try to.op()
            try test(false)
        } catch is Ice.ConnectTimeoutException {
            // Expected.
        }
        try controller.resumeAdapter()
        try timeout.op() // Ensure adapter is active.
    }

    do {
        //
        // Expect success.
        //
        let to = timeout.ice_timeout(-1)
        try controller.holdAdapter(100)
        do {
            try to.op()
        } catch is Ice.ConnectTimeoutException {
            try test(false)
        }
    }
    output.writeLine("ok")

    // The sequence needs to be large enough to fill the write/recv buffers
    let seq = ByteSeq(repeating: 0, count: 2_000_000)

    output.write("testing connection timeout... ")
    do {
        //
        // Expect TimeoutException.
        //
        let to = timeout.ice_timeout(250)
        _ = try connect(to)
        try controller.holdAdapter(-1)
        do {
            try to.sendData(seq)
            try test(false)
        } catch is Ice.TimeoutException {
            // Expected.
        }
        try controller.resumeAdapter()
        try timeout.op() // Ensure adapter is active.
    }

    do {
        //
        // Expect success.
        //
        let to = timeout.ice_timeout(2000)
        try controller.holdAdapter(100)
        do {
            try to.sendData(ByteSeq(repeating: 0, count: 1_000_000))
        } catch is Ice.TimeoutException {
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
            try to.sleep(500)
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

    do {
        //
        // Backward compatible connection timeouts
        //
        let to = timeout.ice_invocationTimeout(-2).ice_timeout(250)
        var con = try connect(to)
        do {
            try to.sleep(750)
            try test(false)
        } catch is Ice.TimeoutException {
            do {
                _ = try con.getInfo()
                try test(false)
            } catch is Ice.TimeoutException {
                // Connection got closed as well.
            }
        }
        try timeout.ice_ping()

        do {
            con = try connect(to)
            try to.sleepAsync(750).wait()
            try test(false)
        } catch is Ice.TimeoutException {
            do {
                _ = try con.getInfo()
                try test(false)
            } catch is Ice.TimeoutException {
                // Connection got closed as well.
            }
        }
        try obj.ice_ping()
    }
    output.writeLine("ok")

    output.write("testing close timeout... ")
    do {
        let to = timeout.ice_timeout(250)
        let connection = try connect(to)
        try controller.holdAdapter(-1)
        try connection.close(.GracefullyWithWait)
        do {
            _ = try connection.getInfo() // getInfo() doesn't throw in the closing state.
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
        try timeout.op() // Ensure adapter is active.
    }
    output.writeLine("ok")

    output.write("testing timeout overrides... ")
    do {
        //
        // Test Ice.Override.Timeout. This property overrides all
        // endpoint timeouts.
        //
        let properties = communicator.getProperties().clone()
        properties.setProperty(key: "Ice.Override.ConnectTimeout", value: "250")
        properties.setProperty(key: "Ice.Override.Timeout", value: "100")
        var initData = Ice.InitializationData()
        initData.properties = properties
        let comm = try helper.initialize(initData)
        var to = try uncheckedCast(prx: comm.stringToProxy(sref)!, type: TimeoutPrx.self)
        _ = try connect(to)
        try controller.holdAdapter(-1)
        do {
            try to.sendData(seq)
            try test(false)
        } catch is Ice.TimeoutException {
            // Expected.
        }
        try controller.resumeAdapter()
        try timeout.op() // Ensure adapter is active.

        //
        // Calling ice_timeout() should have no effect.
        //
        to = to.ice_timeout(1000)
        _ = try connect(to)
        try controller.holdAdapter(-1)
        do {
            try to.sendData(seq)
            try test(false)
        } catch is Ice.TimeoutException {
            // Expected.
        }
        try controller.resumeAdapter()
        try timeout.op() // Ensure adapter is active.
        comm.destroy()
    }

    do {
        //
        // Test Ice.Override.ConnectTimeout.
        //
        let properties = communicator.getProperties().clone()
        properties.setProperty(key: "Ice.Override.ConnectTimeout", value: "250")
        var initData = Ice.InitializationData()
        initData.properties = properties
        let comm = try helper.initialize(initData)
        try controller.holdAdapter(-1)
        var to = try uncheckedCast(prx: comm.stringToProxy(sref)!, type: TimeoutPrx.self)
        do {
            try to.op()
            try test(false)
        } catch is Ice.ConnectTimeoutException {
            // Expected.
        }
        try controller.resumeAdapter()
        try timeout.op() // Ensure adapter is active.

        //
        // Calling ice_timeout() should have no effect on the connect timeout.
        //
        try controller.holdAdapter(-1)
        to = to.ice_timeout(1000)
        do {
            try to.op()
            try test(false)
        } catch is Ice.ConnectTimeoutException {
            // Expected.
        }
        try controller.resumeAdapter()
        try timeout.op() // Ensure adapter is active.

        //
        // Verify that timeout set via ice_timeout() is still used for requests.
        //
        to = to.ice_timeout(250)
        _ = try connect(to)
        try controller.holdAdapter(-1)
        do {
            try to.sendData(seq)
            try test(false)
        } catch is Ice.TimeoutException {
            // Expected.
        }
        try controller.resumeAdapter()
        try timeout.op() // Ensure adapter is active.
        comm.destroy()
    }

    do {
        //
        // Test Ice.Override.CloseTimeout.
        //
        let properties = communicator.getProperties().clone()
        properties.setProperty(key: "Ice.Override.CloseTimeout", value: "100")
        var initData = Ice.InitializationData()
        initData.properties = properties
        let comm = try helper.initialize(initData)
        _ = try comm.stringToProxy(sref)!.ice_getConnection()
        try controller.holdAdapter(-1)
        let begin = DispatchTime.now()
        comm.destroy()
        let elapsed = DispatchTime.now().uptimeNanoseconds - begin.uptimeNanoseconds
        try test((elapsed / 1_000_000) < 1000)
        try controller.resumeAdapter()
    }
    output.writeLine("ok")

    output.write("testing invocation timeouts with collocated calls... ")
    do {
        communicator.getProperties().setProperty(key: "TimeoutCollocated.AdapterId", value: "timeoutAdapter")

        let adapter = try communicator.createObjectAdapter("TimeoutCollocated")
        try adapter.activate()

        let proxy = try uncheckedCast(prx: adapter.addWithUUID(TimeoutDisp(TimeoutI())),
                                      type: TimeoutPrx.self).ice_invocationTimeout(100)
        do {
            try proxy.sleep(500)
            try test(false)
        } catch is Ice.InvocationTimeoutException {}

        do {
            try proxy.sleepAsync(500).wait()
            try test(false)
        } catch is Ice.InvocationTimeoutException {}

        do {
            try proxy.ice_invocationTimeout(-2).ice_ping()
            try proxy.ice_invocationTimeout(-2).ice_pingAsync().wait()
        } catch is Ice.Exception {
            try test(false)
        }

        let batchTimeout = proxy.ice_batchOneway()
        try batchTimeout.ice_ping()
        try batchTimeout.ice_ping()
        try batchTimeout.ice_ping()

        _ = proxy.ice_invocationTimeout(-1).sleepAsync(300) // Keep the server thread pool busy.
        do {
            try batchTimeout.ice_flushBatchRequests()
            try test(false)
        } catch is Ice.InvocationTimeoutException {}

        try batchTimeout.ice_ping()
        try batchTimeout.ice_ping()
        try batchTimeout.ice_ping()

        _ = proxy.ice_invocationTimeout(-1).sleepAsync(300) // Keep the server thread pool busy.
        do {
            try batchTimeout.ice_flushBatchRequestsAsync().wait()
            try test(false)
        } catch is Ice.InvocationTimeoutException {}
        adapter.destroy()
    }
    output.writeLine("ok")
    try controller.shutdown()
}
