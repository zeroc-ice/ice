//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
import Ice

final class MyDerivedClassI: ObjectI<MyDerivedClassTraits>, MyDerivedClass {
    var _ctx: [String: String]

    override init() {
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

    func getContext(current _: Ice.Current) throws -> [String: String] {
        return _ctx
    }

    override func ice_isA(id: String, current: Ice.Current) throws -> Bool {
        _ctx = current.ctx
        return try super.ice_isA(id: id, current: current)
    }

    override func ice_id(current: Ice.Current) throws -> String {
        _ctx = current.ctx
        return try super.ice_id(current: current)
    }

    override func ice_ids(current: Ice.Current) throws -> [String] {
        _ctx = current.ctx
        return try super.ice_ids(current: current)
    }

    override func ice_ping(current: Current) {
        _ctx = current.ctx
    }
}
