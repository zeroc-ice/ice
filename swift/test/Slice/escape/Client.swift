//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import PromiseKit
import TestCommon

class BreakI: `break` {
    func caseAsync(catch _: Int32, current _: Current) -> Promise<Int32> {
        return Promise.value(0)
    }
}

class FuncI: `func` {
    func `public`(current _: Current) throws {}
}

class DoI: `do` {
    func `public`(current _: Current) throws {}

    func caseAsync(catch _: Int32, current _: Current) -> Promise<Int32> {
        return Promise.value(0)
    }
}

public class Client: TestHelperI {
    override public func run(args: [String]) throws {
        let communicator = try initialize(args)
        defer {
            communicator.destroy()
        }
        communicator.getProperties().setProperty(
            key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        try adapter.add(servant: breakDisp(BreakI()), id: Ice.stringToIdentity("test"))
        try adapter.add(servant: funcDisp(FuncI()), id: Ice.stringToIdentity("test1"))
        try adapter.add(servant: doDisp(DoI()), id: Ice.stringToIdentity("test2"))
        try adapter.activate()

        let out = getWriter()
        out.write("testing operation name... ")
        let p = try checkedCast(
            prx: adapter.createProxy(Ice.stringToIdentity("test")), type: breakPrx.self)!
        _ = try p.case(0)
        out.writeLine("ok")

        out.write("testing types... ")
        let e: `continue` = .let

        var g = `guard`()
        g.default = 0

        var d = `defer`()
        d.else = "else"

        let c = `switch`()
        c.if = 0
        c.export = nil
        c.volatile = 0
        try test(c.if == 0)

        let ss = `fileprivate`(repeating: g, count: 1)
        let dd: `for` = ["g": g]
        try test(dd.count == ss.count)

        do {
            if e == .let {
                throw `return`(Int32: 0)
            }
        } catch {
            // Expected
        }

        try test(`is` == 0)
        try test(`throw` == 0)
        try test(`typealias` == 0)
        try test(`internal` == 0)
        try test(`while` == 0)
        try test(`import` == 0)

        out.writeLine("ok")
    }
}
