//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import Foundation

class RemoteCommunicatorI: RemoteCommunicator {
    func createObjectAdapter(acmTimeout: Int32,
                             close: Int32,
                             heartbeat: Int32,
                             current: Ice.Current) throws -> RemoteObjectAdapterPrx? {

        let communicator = current.adapter!.getCommunicator()
        let properties = communicator.getProperties()
        let defaultProtocol = properties.getPropertyWithDefault(key: "Ice.Default.Protocol", value: "tcp")
        let defaultHost = properties.getPropertyWithDefault(key: "Ice.Default.Host", value: "127.0.0.1")

        let name = UUID().uuidString
        if acmTimeout >= 0 {
            try properties.setProperty(key: "\(name).ACM.Timeout", value: "\(acmTimeout)")
        }

        if close >= 0 {
            try properties.setProperty(key: "\(name).ACM.Close", value: "\(close)")
        }

        if heartbeat >= 0 {
            try properties.setProperty(key: "\(name).ACM.Heartbeat", value: "\(heartbeat)")
        }
        try properties.setProperty(key: "\(name).ThreadPool.Size", value: "2")

        let adapter = try communicator.createObjectAdapterWithEndpoints(
            name: name,
            endpoints: "\(defaultProtocol) -h \"\(defaultHost)\"",
            queue: DispatchQueue(label: "Ice.acm.server",
                                 qos: .userInitiated,
                                 attributes: .concurrent))

        return try uncheckedCast(prx: current.adapter!.addWithUUID(RemoteObjectAdapterI(adapter: adapter)),
                                 type: RemoteObjectAdapterPrx.self)
    }

    func shutdown(current: Ice.Current) throws {
        current.adapter!.getCommunicator().shutdown()
    }
}

class RemoteObjectAdapterI: RemoteObjectAdapter {

    var _adapter: Ice.ObjectAdapter
    var _testIntf: TestIntfPrx

    init(adapter: Ice.ObjectAdapter)  throws {
        _adapter = adapter
        _testIntf = try uncheckedCast(prx: adapter.add(servant: TestI(), id: Ice.stringToIdentity("test")),
                                      type: TestIntfPrx.self)
        try _adapter.activate()
    }

    func getTestIntf(current: Current) throws -> TestIntfPrx? {
        return _testIntf
    }

    func activate(current: Current) throws {
        try _adapter.activate()
    }

    func hold(current: Current) throws {
        try _adapter.hold()
    }

    func deactivate(current: Current) throws {
        _adapter.destroy()
    }
}

class TestI: TestIntf {

    class HearbeatCallbackI {
        let _semaphore: DispatchSemaphore
        var _count: Int32

        public init() {
            _count = 0
            _semaphore = DispatchSemaphore(value: 0)
        }

        func hearbeat(conn: Ice.Connection?) {
            _count += 1
            _semaphore.signal()
        }

        func waitForCount(count: Int32) {
            while _count < count {
                _semaphore.wait()
            }
        }
    }

    let _semaphore: DispatchSemaphore
    var _hearbeatCallback: HearbeatCallbackI!

    public init() {
        _semaphore = DispatchSemaphore(value: 0)
    }

    func sleep(seconds: Int32, current: Current) throws {
        _ = _semaphore.wait(timeout: .now() + Double(seconds))
    }

    func sleepAndHold(seconds: Int32, current: Current) throws {
        try current.adapter!.hold()
        _ = _semaphore.wait(timeout: .now() + Double(seconds))
    }

    func interruptSleep(current: Current) throws {
        _semaphore.signal()
    }

    func startHeartbeatCount(current: Current) throws {
        _hearbeatCallback = HearbeatCallbackI()
        try current.con?.setHeartbeatCallback { conn in
            self._hearbeatCallback.hearbeat(conn: conn)
        }
    }

    func waitForHeartbeatCount(count: Int32, current: Current) throws {
        _hearbeatCallback.waitForCount(count: count)
    }
}
