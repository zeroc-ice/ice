//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import Ice
import TestCommon

class CookieI: Cookie {
    func message() -> String {
        return "blahblah"
    }
}

class RouterI: Ice.Router {
    func getClientProxy(current: Ice.Current) throws -> (returnValue: ObjectPrx?, hasRoutingTable: Bool?) {
        return (nil, false)
    }

    func addProxies(proxies: [ObjectPrx?], current: Current) throws -> [ObjectPrx?] {
        return []
    }

    func getServerProxy(current: Ice.Current) throws -> Ice.ObjectPrx? {
        let prx =
          try current.adapter!.getCommunicator().stringToProxy("dummy:tcp -h localhost -p \(_nextPort) -t 30000")
        _nextPort += 1
        return prx
    }

    var _nextPort: Int32 = 23456
}

class ServantLocatorI: Ice.ServantLocator {
    var _helper: TestHelper
    var _deactivated: Bool
    var _router = RouterI()
    var _lock = os_unfair_lock()

    init(helper: TestHelper) {
        _deactivated = false
        _helper = helper
    }

    deinit {
        precondition(_deactivated)
    }

    func locate(_ current: Ice.Current) throws -> (returnValue: Object?, cookie: AnyObject?) {
        try withLock(&_lock) {
            try _helper.test(!_deactivated)
        }

        if current.id.name == "router" {
            return (_router, CookieI())
        }

        try _helper.test(current.id.category == "")
        try _helper.test(current.id.name == "test")

        return (TestI(), CookieI())
    }

    func finished(curr current: Ice.Current, servant: Ice.Object, cookie: Swift.AnyObject) throws {
        try withLock(&_lock) {
            try _helper.test(!_deactivated)
        }

        if current.id.name == "router" {
            return
        }

        try _helper.test((cookie as! Cookie).message() == "blahblah")
    }

    func deactivate(_: String) {
        do {
            try withLock(&_lock) {
                try _helper.test(!_deactivated)
                _deactivated = true
            }
        } catch {
            fatalError("\(error)")
        }
    }
}
