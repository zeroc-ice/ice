// Copyright (c) ZeroC, Inc.

import Ice

actor MyDerivedClassI: MyDerivedClass, Ice.Object {
    var _ctx: [String: String] = [:]

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

    func ice_isA(id: String, current: Ice.Current) async throws -> Bool {
        _ctx = current.ctx
        return try await Ice.DefaultObject<MyDerivedClassTraits>().ice_isA(id: id, current: current)
    }

    func ice_id(current: Ice.Current) async throws -> String {
        _ctx = current.ctx
        return try await Ice.DefaultObject<MyDerivedClassTraits>().ice_id(current: current)
    }

    func ice_ids(current: Ice.Current) async throws -> [String] {
        _ctx = current.ctx
        return try await Ice.DefaultObject<MyDerivedClassTraits>().ice_ids(current: current)
    }

    func ice_ping(current: Current) async {
        _ctx = current.ctx
    }
}
