//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
import Ice

class MyDerivedClassI: MyDerivedClass {
    var _ctx: [String: String]

    public init() {
        _ctx = [String: String]()
    }

    public func echo(obj: Ice.ObjectPrx?, current _: Ice.Current) throws -> Ice.ObjectPrx? {
        return obj
    }

    public func shutdown(current: Ice.Current) throws {
        guard let adapter = current.adapter else {
            precondition(false)
        }
        adapter.getCommunicator().shutdown()
    }

    public func getContext(current: Ice.Current) throws -> [String: String] {
        return _ctx
    }

    public func ice_isA(s: String, current: Ice.Current) throws -> Bool {
        _ctx = current.ctx
        return try ice_ids(current: current).contains(s)
    }
}
