// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

class BreakI: `break` {
    func `case`(class _: Int32, current _: Current) async throws -> Int32 {
        return 0
    }
}

class DoI: `do` {
    func `public`(current _: Current) async throws {}

    func `case`(class _: Int32, current _: Current) async throws -> Int32 {
        return 0
    }

    func goto(
        `if`: `continue`,
        d: `guard`,
        `private`: `switch`?,
        mutable: doPrx?,
        foo: breakPrx?,
        not: `switch`?,
        or: Swift.Int64,
        current: Ice.Current
    ) async throws -> `guard` {
        var g = `guard`()
        g.default = 79
        return g
    }
}

public class Client: TestHelperI {
    override public func run(args: [String]) async throws {
        let communicator = try initialize(args)
        defer {
            communicator.destroy()
        }
        communicator.getProperties().setProperty(
            key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        try adapter.add(servant: breakDisp(BreakI()), id: Ice.Identity(name: "test"))
        try adapter.add(servant: doDisp(DoI()), id: Ice.stringToIdentity("test1"))
        try adapter.activate()

        let out = getWriter()
        out.write("testing operation name... ")
        let p = try await checkedCast(
            prx: adapter.createProxy(Ice.Identity(name: "test")), type: breakPrx.self)!
        _ = try await p.case(0)
        out.writeLine("ok")

        out.write("testing types... ")
        let e: `continue` = .myFirstEnumerator

        var g = `guard`()
        g.default = 0

        let c = `switch`()
        c.`remappedExport` = nil
        c.remappedVolatile = 0
        try test(c.remappedVolatile == 0)

        let ss = `fileprivate`(repeating: g, count: 1)
        let dd: `for` = ["g": g]
        try test(dd.count == ss.count)

        do {
            if e == .myFirstEnumerator {
                throw `return`(Int32: 0)
            }
        } catch {
            // Expected
        }

        try test(`typealias` == 0)

        out.writeLine("ok")
    }
}
