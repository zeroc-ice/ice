// Copyright (c) ZeroC, Inc.

import Foundation
import Ice
import Synchronization
import TestCommon

private struct TestI: TestIntf {
    func transient(current: Ice.Current) async throws {
        let communicator = current.adapter.getCommunicator()

        let adapter = try communicator.createObjectAdapterWithEndpoints(
            name: "TransientTestAdapter",
            endpoints: "default")
        try adapter.activate()
        adapter.destroy()
    }

    func deactivate(current: Ice.Current) async throws {
        current.adapter.deactivate()
        try await Task.sleep(for: .milliseconds(100))
    }
}

final class Cookie {
    func message() -> String {
        return "blahblah"
    }
}

private struct RouterI: Ice.Router {
    func getClientProxy(current _: Ice.Current) async throws -> (
        returnValue: ObjectPrx?, hasRoutingTable: Bool?
    ) {
        return (nil, false)
    }

    func addProxies(proxies _: [ObjectPrx?], current _: Current) async throws -> [ObjectPrx?] {
        return []
    }

    func getServerProxy(current: Ice.Current) async throws -> Ice.ObjectPrx? {
        return try makeProxy(
            communicator: current.adapter.getCommunicator(),
            proxyString: "dummy:tcp -h localhost -p 23456 -t 30000",
            type: Ice.ObjectPrx.self)
    }
}

final class ServantLocatorI: Ice.ServantLocator {
    private let _helper: TestHelper
    private let _deactivated = Mutex<Bool>(false)
    private let _router = RouterI()

    init(helper: TestHelper) {
        _helper = helper
    }

    deinit {
        precondition(_deactivated.withLock { $0 }, "ServantLocatorI deactivated must be true before deinit")
    }

    func locate(_ current: Ice.Current) throws -> sending (returnValue: Dispatcher?, cookie: AnyObject?) {
        try _deactivated.withLock {
            try _helper.test(!$0)
        }

        if current.id.name == "router" {
            return (_router, Cookie())
        }

        try _helper.test(current.id.category == "")
        try _helper.test(current.id.name == "test")

        return (TestI(), Cookie())
    }

    func finished(curr current: Ice.Current, servant _: Ice.Dispatcher, cookie: AnyObject?) throws {
        try _deactivated.withLock {
            try _helper.test(!$0)
        }

        if current.id.name == "router" {
            return
        }

        try _helper.test((cookie as! Cookie).message() == "blahblah")
    }

    func deactivate(_: String) {
        do {
            try _deactivated.withLock {
                try _helper.test(!$0)
                $0 = true
            }
        } catch {
            fatalError("\(error)")
        }
    }
}
