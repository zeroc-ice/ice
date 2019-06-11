//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
import Ice
import PromiseKit

final class MyDerivedClassI: MyDerivedClass, Object {
    var _ctx: [String: String]

    init() {
        _ctx = [String: String]()
    }

    func echoAsync(obj: Ice.ObjectPrx?, current _: Ice.Current) -> PromiseKit.Promise<Ice.ObjectPrx?> {
        return Promise.value(obj)
    }

    func shutdownAsync(current: Ice.Current) -> PromiseKit.Promise<Void> {
        guard let adapter = current.adapter else {
            fatalError()
        }
        adapter.getCommunicator().shutdown()
        return Promise.value(())
    }

    func getContextAsync(current: Ice.Current) -> PromiseKit.Promise<[String: String]> {
        return Promise.value(_ctx)
    }

    func ice_isA(s: String, current: Ice.Current) -> Bool {
        _ctx = current.ctx
        return MyDerivedClassDisp.staticIds.contains(s)
    }

    func ice_ids(current: Ice.Current) -> [String] {
        _ctx = current.ctx
        return MyDerivedClassDisp.staticIds
    }

    func ice_id(current: Ice.Current) -> String {
        _ctx = current.ctx
        return MyDerivedClassDisp.staticId
    }

    func ice_ping(current: Ice.Current) {
        _ctx = current.ctx
    }
}
