// Copyright (c) ZeroC, Inc.

import Ice

final class MyDerivedClassI: ObjectI<MyDerivedClassTraits>, MyDerivedClass {
    var _ctx: [String: String]

    override init() {
        _ctx = [String: String]()
    }

    func echo(obj: Ice.ObjectPrx?, current _: Ice.Current) async throws -> Ice.ObjectPrx? {
        return obj
    }

    func shutdown(current: Ice.Current) async throws {
        let adapter = current.adapter
        adapter.getCommunicator().shutdown()
    }

    func getContext(current _: Ice.Current) async throws -> [String: String] {
        return _ctx
    }

    override func ice_isA(id: String, current: Ice.Current) async throws -> Bool {
        _ctx = current.ctx
        return try await super.ice_isA(id: id, current: current)
    }

    override func ice_id(current: Ice.Current) async throws -> String {
        _ctx = current.ctx
        return try await super.ice_id(current: current)
    }

    override func ice_ids(current: Ice.Current) async throws -> [String] {
        _ctx = current.ctx
        return try await super.ice_ids(current: current)
    }

    override func ice_ping(current: Current) async {
        _ctx = current.ctx
    }
}
