//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
import Ice
import PromiseKit

final class MyDerivedClassI: ObjectI<MyDerivedClassTraits>, MyDerivedClass {
    var _ctx: [String: String]

    override init() {
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

    func getContextAsync(current _: Ice.Current) -> PromiseKit.Promise<[String: String]> {
        return Promise.value(_ctx)
    }

    override func ice_isA(id: String, current: Ice.Current) throws -> Bool {
        _ctx = current.ctx
        return try super.ice_isA(id: id, current: current)
    }

    override func ice_ids(current: Ice.Current) throws -> [String] {
        _ctx = current.ctx
        return try super.ice_ids(current: current)
    }

    override func ice_id(current: Ice.Current) throws -> String {
        _ctx = current.ctx
        return try super.ice_id(current: current)
    }

    override func ice_ping(current: Ice.Current) {
        _ctx = current.ctx
    }
}
