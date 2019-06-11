//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
import Ice

final class MyDerivedClassI: MyDerivedClass, Object {
    var _ctx: [String: String]

    init() {
        _ctx = [String: String]()
    }

    func echo(obj: Ice.ObjectPrx?, current _: Ice.Current) throws -> Ice.ObjectPrx? {
        return obj
    }

    func shutdown(current: Ice.Current) throws {
        guard let adapter = current.adapter else {
            fatalError()
        }
        adapter.getCommunicator().shutdown()
    }

    func getContext(current: Ice.Current) throws -> [String: String] {
        return _ctx
    }

    func ice_isA(s: String, current: Ice.Current) -> Bool {
        _ctx = current.ctx
        return MyDerivedClassDisp.staticIds.contains(s)
    }

    func ice_id(current: Ice.Current) -> String {
        _ctx = current.ctx
        return MyDerivedClassDisp.staticId
    }

    func ice_ids(current: Ice.Current) -> [String] {
        _ctx = current.ctx
        return MyDerivedClassDisp.staticIds
    }

    func ice_ping(current: Current) {
        _ctx = current.ctx
    }
}
