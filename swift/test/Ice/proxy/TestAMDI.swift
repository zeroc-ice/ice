//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
import Ice
import PromiseKit

class MyDerivedClassI: MyDerivedClass {
    var _ctx: [String: String]

    init() {
        _ctx = [String: String]()
    }

    func echoAsync(obj: Ice.ObjectPrx?, current _: Ice.Current) -> PromiseKit.Promise<Ice.ObjectPrx?> {
        return Promise.value(obj)
    }

    func shutdownAsync(current: Ice.Current) -> PromiseKit.Promise<Void> {
        guard let adapter = current.adapter else {
            precondition(false)
        }
        adapter.getCommunicator().shutdown()
        return Promise.value(())
    }

    func getContextAsync(current: Ice.Current) -> PromiseKit.Promise<[String: String]> {
        return Promise.value(_ctx)
    }

    func ice_isA(s: String, current: Ice.Current) throws -> Bool {
        _ctx = current.ctx
        return try ice_ids(current: current).contains(s)
    }
}
