//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import Ice

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
            properties.setProperty(key: "\(name).ACM.Timeout", value: "\(acmTimeout)")
        }

        if close >= 0 {
            properties.setProperty(key: "\(name).ACM.Close", value: "\(close)")
        }

        if heartbeat >= 0 {
            properties.setProperty(key: "\(name).ACM.Heartbeat", value: "\(heartbeat)")
        }
        properties.setProperty(key: "\(name).ThreadPool.Size", value: "2")

        let adapter = try communicator.createObjectAdapterWithEndpoints(
            name: name,
            endpoints: "\(defaultProtocol) -h \"\(defaultHost)\""
        )

        return try uncheckedCast(
            prx: current.adapter!.addWithUUID(RemoteObjectAdapterDisp(RemoteObjectAdapterI(adapter: adapter))),
            type: RemoteObjectAdapterPrx.self
        )
    }

    func shutdown(current: Ice.Current) throws {
        current.adapter!.getCommunicator().shutdown()
    }
}

class RemoteObjectAdapterI: RemoteObjectAdapter {
    var _adapter: Ice.ObjectAdapter
    var _testIntf: TestIntfPrx

    init(adapter: Ice.ObjectAdapter) throws {
        _adapter = adapter
        _testIntf = try uncheckedCast(prx: adapter.add(servant: TestIntfDisp(TestI()),
                                                       id: Ice.stringToIdentity("test")),
                                      type: TestIntfPrx.self)
        try _adapter.activate()
    }

    func getTestIntf(current _: Current) -> TestIntfPrx? {
        return _testIntf
    }

    func activate(current _: Current) throws {
        try _adapter.activate()
    }

    func hold(current _: Current) {
        _adapter.hold()
    }

    func deactivate(current _: Current) {
        _adapter.destroy()
    }
}

class TestI: TestIntf {
    let _semaphore: DispatchSemaphore
    var _hearbeatCallback: DispatchSemaphore!

    public init() {
        _semaphore = DispatchSemaphore(value: 0)
    }

    func sleep(seconds: Int32, current _: Current) {
        _ = _semaphore.wait(timeout: .now() + Double(seconds))
    }

    func sleepAndHold(seconds: Int32, current: Current) {
        current.adapter!.hold()
        _ = _semaphore.wait(timeout: .now() + Double(seconds))
    }

    func interruptSleep(current _: Current) {
        _semaphore.signal()
    }

    func startHeartbeatCount(current: Current) {
        _hearbeatCallback = DispatchSemaphore(value: 0)
        current.con?.setHeartbeatCallback { _ in
            self._hearbeatCallback.signal()
        }
    }

    func waitForHeartbeatCount(count: Int32, current _: Current) {
        for _ in 0..<count {
            self._hearbeatCallback.wait()
        }
    }
}
