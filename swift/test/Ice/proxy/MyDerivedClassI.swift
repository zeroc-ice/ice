//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
import Ice

class MyDerivedClassI: MyDerivedClass {
    var _ctx: [String: String]

    init() {
        _ctx = [String: String]()
    }

    func echo(obj: Ice.ObjectPrx?, current _: Ice.Current) throws -> Ice.ObjectPrx? {
        return obj
    }

    func shutdown(current: Ice.Current) throws {
        guard let adapter = current.adapter else {
            precondition(false)
        }
        adapter.getCommunicator().shutdown()
    }

    func getContext(current: Ice.Current) throws -> [String: String] {
        return _ctx
    }

    func ice_isA(s: String, current: Ice.Current) throws -> Bool {
        _ctx = current.ctx
        return try ice_ids(current: current).contains(s)
    }
}
