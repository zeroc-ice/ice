// Copyright (c) ZeroC, Inc.

import Ice

actor MyDerivedClassI: MyDerivedClass, Ice.Object {
    var _ctx: [String: String] = [:]

    func echo(obj: Ice.ObjectPrx?, current _: Ice.Current) -> Ice.ObjectPrx? {
        return obj
    }

    func shutdown(current: Ice.Current) {
        let adapter = current.adapter
        adapter.getCommunicator().shutdown()
    }

    func getContext(current _: Ice.Current) -> [String: String] {
        return _ctx
    }

    func ice_isA(id: String, current: Ice.Current) -> Bool {
        _ctx = current.ctx
        return Ice.DefaultObject<MyDerivedClassTraits>().ice_isA(id: id, current: current)
    }

    func ice_id(current: Ice.Current) -> String {
        _ctx = current.ctx
        return Ice.DefaultObject<MyDerivedClassTraits>().ice_id(current: current)
    }

    func ice_ids(current: Ice.Current) -> [String] {
        _ctx = current.ctx
        return Ice.DefaultObject<MyDerivedClassTraits>().ice_ids(current: current)
    }

    func ice_ping(current: Current) {
        _ctx = current.ctx
    }
}
