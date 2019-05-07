//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import TestCommon
import Ice
import PromiseKit

class breakI: `break` {
    func caseAsync(catch: Int32, current: Current) -> Promise<Int32> {
        return Promise.value(0)
    }
}

class funcI: `func` {
    func `public`(current: Current) throws {
    }
}

class doI: `do` {
    func `public`(current: Current) throws {
    }

    func caseAsync(catch: Int32, current: Current) -> Promise<Int32> {
        return Promise.value(0)
    }
}

public class Client: TestHelperI {
    public override func run(args: [String]) throws {
        let communicator = try self.initialize(args)
        defer {
            communicator.destroy()
        }
        communicator.getProperties().setProperty(key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        try adapter.add(servant: breakI(), id: Ice.stringToIdentity("test"))
        try adapter.add(servant: funcI(), id: Ice.stringToIdentity("test1"))
        try adapter.add(servant: doI(), id: Ice.stringToIdentity("test2"))
        try adapter.activate()

        let out = getWriter()
        out.write("testing operation name... ")
        let p = try checkedCast(prx: adapter.createProxy(Ice.stringToIdentity("test")), type: breakPrx.self)!
        _ = try p.case(0)
        out.writeLine("ok")

        out.write("testing types... ")
        let e: `continue` = .let

        var g: `guard` = `guard`()
        g.default = 0

        var d: `defer` = `defer`()
        d.else = "else"

        let c: `switch` = `switch`()
        c.if = 0
        c.export = nil
        c.volatile = 0
        try test(c.if == 0)

        let ss: `fileprivate` = `fileprivate`(repeating: g, count: 1)
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
        //try test(`self` == 0)
        try test(`throw` == 0)
        try test(`typealias` == 0)
        try test(`internal` == 0)
        try test(`while` == 0)
        try test(`import` == 0)

        out.writeLine("ok")
    }
}
