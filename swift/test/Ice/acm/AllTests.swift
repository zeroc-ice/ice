//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import Ice
import PromiseKit
import TestCommon

class LoggerI: Ice.Logger {
    var _name: String
    var _output: TextWriter
    var _started: Bool
    var _messages: [String]
    var _dateFormat: DateFormatter
    var _timeFormat: DateFormatter
    var _lock = os_unfair_lock()

    init(name: String, output: TextWriter) {
        _name = name
        _output = output
        _started = false
        _messages = [String]()

        _dateFormat = DateFormatter()
        _dateFormat.setLocalizedDateFormatFromTemplate("d")

        _timeFormat = DateFormatter()
        _timeFormat.setLocalizedDateFormatFromTemplate("HH:mm:ss:fff")
    }

    func start() {
        withLock(&_lock) {
            _started = true
            dump()
        }
    }

    func print(_ msg: String) {
        withLock(&_lock) {
            _messages.append(msg)
            if _started {
                dump()
            }
        }
    }

    func trace(category: String, message: String) {
        withLock(&_lock) {
            var s = _name
            s += " "
            s += _dateFormat.string(from: Date())
            s += " "
            s += _timeFormat.string(from: Date())
            s += " "
            s += "["
            s += category
            s += "] "
            s += message
            _messages.append(s)
            if _started {
                dump()
            }
        }
    }

    func warning(_ message: String) {
        withLock(&_lock) {
            var s = _name
            s += " "
            s += _dateFormat.string(from: Date())
            s += " "
            s += _timeFormat.string(from: Date())
            s += " warning : "
            s += message
            _messages.append(s)
            if _started {
                dump()
            }
        }
    }

    func error(_ message: String) {
        withLock(&_lock) {
            var s = _name
            s += " "
            s += _dateFormat.string(from: Date())
            s += " "
            s += _timeFormat.string(from: Date())
            s += " error : "
            s += message
            _messages.append(s)
            if _started {
                dump()
            }
        }
    }

    func getPrefix() -> String {
        return ""
    }

    func cloneWithPrefix(_: String) -> Ice.Logger {
        return self
    }

    func dump() {
        for line in _messages {
            _output.writeLine(line)
        }
        _messages = []
    }
}

class TestCase {
    var _name: String
    var _com: RemoteCommunicatorPrx
    var _msg: String!
    var _output: TextWriter
    var _logger: LoggerI
    var _helper: TestHelper

    var _clientACMTimeout: Int32
    var _clientACMClose: Int32
    var _clientACMHeartbeat: Int32

    var _serverACMTimeout: Int32
    var _serverACMClose: Int32
    var _serverACMHeartbeat: Int32

    var _heartbeat: Int32
    var _closed: Bool
    var _semaphore: DispatchSemaphore
    var _lock = os_unfair_lock()
    var _adapter: RemoteObjectAdapterPrx!
    var _communicator: Ice.Communicator!

    var _queue: DispatchQueue
    var _group: DispatchGroup

    init(name: String, com: RemoteCommunicatorPrx, helper: TestHelper) {
        _name = name
        _com = com
        _output = helper.getWriter()
        _logger = LoggerI(name: _name, output: _output)
        _helper = helper

        _clientACMTimeout = -1
        _clientACMClose = -1
        _clientACMHeartbeat = -1

        _serverACMTimeout = -1
        _serverACMClose = -1
        _serverACMHeartbeat = -1

        _heartbeat = 0
        _closed = false
        _semaphore = DispatchSemaphore(value: 0)

        _queue = DispatchQueue(label: name)
        _group = DispatchGroup()
    }

    func initialize() throws {
        _adapter = try _com.createObjectAdapter(acmTimeout: _serverACMTimeout,
                                                close: _serverACMClose,
                                                heartbeat: _serverACMHeartbeat)

        let properties = _com.ice_getCommunicator().getProperties().clone()
        properties.setProperty(key: "Ice.ACM.Timeout", value: "2")
        if _clientACMTimeout >= 0 {
            properties.setProperty(key: "Ice.ACM.Client.Timeout", value: "\(_clientACMTimeout)")
        }

        if _clientACMClose >= 0 {
            properties.setProperty(key: "Ice.ACM.Client.Close", value: "\(_clientACMClose)")
        }

        if _clientACMHeartbeat >= 0 {
            properties.setProperty(key: "Ice.ACM.Client.Heartbeat", value: "\(_clientACMHeartbeat)")
        }
        //try properties.setProperty(key: "Ice.Trace.Protocol", value: "2")
        //try properties.setProperty(key: "Ice.Trace.Network", value: "2")

        var initData = Ice.InitializationData()
        initData.properties = properties
        initData.logger = _logger
        _communicator = try _helper.initialize(initData)
    }

    func start() {
        _queue.async {
            self._group.enter()
            self.run()
            self._group.leave()
        }
    }

    func destroy() throws {
        try _adapter.deactivate()
        _communicator.destroy()
    }

    func join() throws {
        _output.write("testing \(_name)... ")
        _logger.start()
        _group.wait()
        if let msg = _msg {
            _output.writeLine("failed! \(msg)")
            throw Ice.RuntimeError(msg)
        } else {
            _output.writeLine("ok")
        }
    }

    func run() {
        do {
            let str = try _adapter.getTestIntf()!.ice_toString()
            let proxy = try uncheckedCast(prx: _communicator.stringToProxy(str)!,
                                          type: TestIntfPrx.self)
            try proxy.ice_getConnection()!.setCloseCallback { _ in
                withLock(&self._lock) {
                    self._closed = true
                    self._semaphore.signal()
                }
            }

            try proxy.ice_getConnection()!.setHeartbeatCallback { _ in
                withLock(&self._lock) {
                    self._heartbeat += 1
                }
            }

            try runTestCase(adapter: _adapter, proxy: proxy)
        } catch let e {
            _msg = "unexpected exception:\n\(e)"
        }
    }

    func waitForClosed() {
        do {
            os_unfair_lock_lock(&_lock)
            defer {
                os_unfair_lock_unlock(&_lock)
            }

            if _closed {
                return
            }
        }

        if _semaphore.wait(timeout: .now() + Double(2)) == .timedOut {
            precondition(false) // Waited for more than 2s for close, something's wrong.
        }
        precondition(_closed)
    }

    func runTestCase(adapter _: RemoteObjectAdapterPrx, proxy _: TestIntfPrx) throws {
        precondition(false, "Abstract Method")
    }

    func setClientACM(timeout: Int32, close: Int32, heartbeat: Int32) {
        _clientACMTimeout = timeout
        _clientACMClose = close
        _clientACMHeartbeat = heartbeat
    }

    func setServerACM(timeout: Int32, close: Int32, heartbeat: Int32) {
        _serverACMTimeout = timeout
        _serverACMClose = close
        _serverACMHeartbeat = heartbeat
    }
}

class InvocationHeartbeatTest: TestCase {
    init(com: RemoteCommunicatorPrx, helper: TestHelper) {
        super.init(name: "invocation heartbeat", com: com, helper: helper)
        setServerACM(timeout: 1, close: -1, heartbeat: -1) // Faster ACM to make sure we receive enough ACM hearbeats
    }

    override func runTestCase(adapter _: RemoteObjectAdapterPrx, proxy: TestIntfPrx) throws {
        try proxy.sleep(4)

        try withLock(&_lock) {
            try _helper.test(self._heartbeat >= 4)
        }
    }
}

class InvocationHeartbeatOnHoldTest: TestCase {
    init(com: RemoteCommunicatorPrx, helper: TestHelper) {
        super.init(name: "invocation with heartbeat on hold", com: com, helper: helper)
        // Use default ACM configuration.
    }

    override func runTestCase(adapter: RemoteObjectAdapterPrx, proxy: TestIntfPrx) throws {
        do {
            // When the OA is put on hold, connections shouldn't
            // send heartbeats, the invocation should therefore
            // fail.
            try proxy.sleepAndHold(10)
            try _helper.test(false)
        } catch is Ice.ConnectionTimeoutException {
            try adapter.activate()
            try proxy.interruptSleep()
            waitForClosed()
        }
    }
}

class InvocationNoHeartbeatTest: TestCase {
    init(com: RemoteCommunicatorPrx, helper: TestHelper) {
        super.init(name: "invocation with no heartbeat", com: com, helper: helper)
        setServerACM(timeout: 2, close: 2, heartbeat: 0) // Disable heartbeat on invocations
    }

    override func runTestCase(adapter _: RemoteObjectAdapterPrx, proxy: TestIntfPrx) throws {
        do {
            // Heartbeats are disabled on the server, the
            // invocation should fail since heartbeats are
            // expected.
            try proxy.sleep(10)
            try _helper.test(false)
        } catch is Ice.ConnectionTimeoutException {
            try proxy.interruptSleep()
            waitForClosed()
            try withLock(&_lock) {
                try _helper.test(_heartbeat == 0)
            }
        }
    }
}

class InvocationHeartbeatCloseOnIdleTest: TestCase {
    init(com: RemoteCommunicatorPrx, helper: TestHelper) {
        super.init(name: "invocation with no heartbeat and close on idle", com: com, helper: helper)
        setClientACM(timeout: 1, close: 1, heartbeat: 0) // Only close on idle.
        setServerACM(timeout: 1, close: 2, heartbeat: 0) // Disable heartbeat on invocations
    }

    override func runTestCase(adapter _: RemoteObjectAdapterPrx, proxy: TestIntfPrx) throws {
        // No close on invocation, the call should succeed this
        // time.
        try proxy.sleep(3)
        try withLock(&_lock) {
            try _helper.test(self._heartbeat == 0)
            try _helper.test(!self._closed)
        }
    }
}

class CloseOnIdleTest: TestCase {
    init(com: RemoteCommunicatorPrx, helper: TestHelper) {
        super.init(name: "close on idle", com: com, helper: helper)
        setClientACM(timeout: 1, close: 1, heartbeat: 0) // Only close on idle
    }

    override func runTestCase(adapter _: RemoteObjectAdapterPrx, proxy _: TestIntfPrx) throws {
        Thread.sleep(forTimeInterval: 3) // Idle for 3 seconds

        waitForClosed()
        try withLock(&_lock) {
            try _helper.test(self._heartbeat == 0)
        }
    }
}

class CloseOnInvocationTest: TestCase {
    init(com: RemoteCommunicatorPrx, helper: TestHelper) {
        super.init(name: "close on invocation", com: com, helper: helper)
        setClientACM(timeout: 1, close: 2, heartbeat: 0) // Only close on invocation
    }

    override func runTestCase(adapter _: RemoteObjectAdapterPrx, proxy _: TestIntfPrx) throws {
        Thread.sleep(forTimeInterval: 3) // Idle for 3 seconds
        try withLock(&_lock) {
            try _helper.test(self._heartbeat == 0)
            try _helper.test(!self._closed)
        }
    }
}

class CloseOnIdleAndInvocationTest: TestCase {
    init(com: RemoteCommunicatorPrx, helper: TestHelper) {
        super.init(name: "close on idle and invocation", com: com, helper: helper)
        setClientACM(timeout: 1, close: 3, heartbeat: 0) // Only close on idle and invocation
    }

    override func runTestCase(adapter: RemoteObjectAdapterPrx, proxy _: TestIntfPrx) throws {
        //
        // Put the adapter on hold. The server will not respond to
        // the graceful close. This allows to test whether or not
        // the close is graceful or forceful.
        //
        try adapter.hold()
        Thread.sleep(forTimeInterval: 3) // Idle for 3 seconds

        try withLock(&_lock) {
            try _helper.test(self._heartbeat == 0)
            try _helper.test(!self._closed) // Not closed yet because of graceful close.
        }

        try adapter.activate()
        Thread.sleep(forTimeInterval: 1)

        waitForClosed()
    }
}

class ForcefulCloseOnIdleAndInvocationTest: TestCase {
    init(com: RemoteCommunicatorPrx, helper: TestHelper) {
        super.init(name: "forceful close on idle and invocation", com: com, helper: helper)
        setClientACM(timeout: 1, close: 4, heartbeat: 0) // Only close on idle and invocation
    }

    override func runTestCase(adapter: RemoteObjectAdapterPrx, proxy _: TestIntfPrx) throws {
        try adapter.hold()
        Thread.sleep(forTimeInterval: 3) // Idle for 3 seconds
        waitForClosed()
        try withLock(&_lock) {
            try _helper.test(self._heartbeat == 0)
        }
    }
}

class HeartbeatOnIdleTest: TestCase {
    init(com: RemoteCommunicatorPrx, helper: TestHelper) {
        super.init(name: "heartbeat on idle", com: com, helper: helper)
        setServerACM(timeout: 1, close: -1, heartbeat: 2) // Enable server heartbeats.
    }

    override func runTestCase(adapter _: RemoteObjectAdapterPrx, proxy _: TestIntfPrx) throws {
        Thread.sleep(forTimeInterval: 3)
        try withLock(&_lock) {
            try _helper.test(_heartbeat >= 3)
        }
    }
}

class HeartbeatAlwaysTest: TestCase {
    init(com: RemoteCommunicatorPrx, helper: TestHelper) {
        super.init(name: "heartbeat always", com: com, helper: helper)
        setServerACM(timeout: 1, close: -1, heartbeat: 3) // Enable server heartbeats.
    }

    override func runTestCase(adapter _: RemoteObjectAdapterPrx, proxy: TestIntfPrx) throws {
        for _ in 0 ..< 10 {
            try proxy.ice_ping()
            Thread.sleep(forTimeInterval: 0.3)
        }

        try withLock(&_lock) {
            try _helper.test(self._heartbeat >= 3)
        }
    }
}

class HeartbeatManualTest: TestCase {
    init(com: RemoteCommunicatorPrx, helper: TestHelper) {
        super.init(name: "manual heartbeats", com: com, helper: helper)
        //
        // Disable heartbeats.
        //
        setClientACM(timeout: 10, close: -1, heartbeat: 0)
        setServerACM(timeout: 10, close: -1, heartbeat: 0)
    }

    override func runTestCase(adapter _: RemoteObjectAdapterPrx, proxy: TestIntfPrx) throws {
        try proxy.startHeartbeatCount()
        let con = try proxy.ice_getConnection()!
        try con.heartbeat()
        try con.heartbeat()
        try con.heartbeat()
        try con.heartbeat()
        try con.heartbeat()
        try proxy.waitForHeartbeatCount(5)
    }
}

class SetACMTest: TestCase {
    init(com: RemoteCommunicatorPrx, helper: TestHelper) {
        super.init(name: "setACM/getACM", com: com, helper: helper)
        setClientACM(timeout: 15, close: 4, heartbeat: 0)
    }

    override func runTestCase(adapter _: RemoteObjectAdapterPrx, proxy: TestIntfPrx) throws {
        guard let con = proxy.ice_getCachedConnection() else {
            precondition(false)
        }

        var acm = con.getACM()
        try _helper.test(acm.timeout == 15)
        try _helper.test(acm.close == .CloseOnIdleForceful)
        try _helper.test(acm.heartbeat == .HeartbeatOff)

        con.setACM(timeout: nil, close: nil, heartbeat: nil)
        acm = con.getACM()
        try _helper.test(acm.timeout == 15)
        try _helper.test(acm.close == .CloseOnIdleForceful)
        try _helper.test(acm.heartbeat == .HeartbeatOff)

        con.setACM(timeout: 1, close: .CloseOnInvocationAndIdle, heartbeat: .HeartbeatAlways)
        acm = con.getACM()
        try _helper.test(acm.timeout == 1)
        try _helper.test(acm.close == .CloseOnInvocationAndIdle)
        try _helper.test(acm.heartbeat == .HeartbeatAlways)

        try proxy.startHeartbeatCount()
        try proxy.waitForHeartbeatCount(2)

        let p1 = Promise<Void> { seal in
            do {
                try con.setCloseCallback { _ in
                    seal.fulfill(())
                }
            } catch {
                seal.reject(error)
            }
        }
        try con.close(.Gracefully)
        try p1.wait()

        do {
            try con.throwException()
            try _helper.test(false)
        } catch is Ice.ConnectionManuallyClosedException {}

        try Promise<Void> { seal in
            do {
                try con.setCloseCallback { _ in
                    seal.fulfill(())
                }
            } catch {
                seal.reject(error)
            }
        }.wait()

        con.setHeartbeatCallback { _ in
            precondition(false)
        }
    }
}

func allTests(helper: TestHelper) throws {
    let writer = helper.getWriter()
    let communicator = helper.communicator()

    let com = try uncheckedCast(prx: communicator.stringToProxy("communicator:\(helper.getTestEndpoint(num: 0))")!,
                                type: RemoteCommunicatorPrx.self)

    let tests: [TestCase] = [
        InvocationHeartbeatTest(com: com, helper: helper),
        InvocationHeartbeatOnHoldTest(com: com, helper: helper),
        InvocationNoHeartbeatTest(com: com, helper: helper),
        InvocationHeartbeatCloseOnIdleTest(com: com, helper: helper),

        CloseOnIdleTest(com: com, helper: helper),
        CloseOnInvocationTest(com: com, helper: helper),
        CloseOnIdleAndInvocationTest(com: com, helper: helper),
        ForcefulCloseOnIdleAndInvocationTest(com: com, helper: helper),

        HeartbeatOnIdleTest(com: com, helper: helper),
        HeartbeatAlwaysTest(com: com, helper: helper),
        HeartbeatManualTest(com: com, helper: helper),
        SetACMTest(com: com, helper: helper)
    ]

    for t in tests {
        try t.initialize()
    }

    for t in tests {
        t.start()
    }

    for t in tests {
        try t.join()
    }

    for t in tests {
        try t.destroy()
    }

    writer.write("shutting down... ")
    try com.shutdown()
    writer.writeLine("ok")
}
