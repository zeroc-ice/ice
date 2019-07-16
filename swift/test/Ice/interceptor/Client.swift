//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import PromiseKit
import TestCommon

public class Client: TestHelperI {
    private func runTest(prx: MyObjectPrx, interceptor: InterceptorI) throws {
        do {
            let out = getWriter()
            out.write("testing simple interceptor... ")
            try test(interceptor.lastOperation == nil)
            try test(!interceptor.lastStatus)
            try prx.ice_ping()
            try test(interceptor.lastOperation == "ice_ping")
            try test(!interceptor.lastStatus)
            let typeId = try prx.ice_id()
            try test(interceptor.lastOperation == "ice_id")
            try test(!interceptor.lastStatus)
            try test(prx.ice_isA(id: typeId))
            try test(interceptor.lastOperation == "ice_isA")
            try test(!interceptor.lastStatus)
            try test(prx.add(x: 33, y: 12) == 45)
            try test(interceptor.lastOperation == "add")
            try test(!interceptor.lastStatus)
            out.writeLine("ok")

            out.write("testing user exception... ")
            do {
                _ = try prx.badAdd(x: 33, y: 12)
                try test(false)
            } catch is InvalidInputException {
                // Expected
            }
            try test(interceptor.lastOperation == "badAdd")
            try test(!interceptor.lastStatus)
            out.writeLine("ok")
            out.write("testing ONE... ")
            do {
                _ = try prx.notExistAdd(x: 33, y: 12)
                try test(false)
            } catch is ObjectNotExistException {
                // Expected
            }
            try test(interceptor.lastOperation == "notExistAdd")
            try test(!interceptor.lastStatus)
            out.writeLine("ok")
        }
    }

    private func runAmdTest(prx: MyObjectPrx, interceptor: InterceptorI, out: TextWriter) throws {
        do {
            out.write("testing simple interceptor... ")
            try test(interceptor.lastOperation == nil)
            try test(!interceptor.lastStatus)
            try test(prx.amdAdd(x: 33, y: 12) == 45)
            try test(interceptor.lastOperation == "amdAdd")
            try test(interceptor.lastStatus)
            out.writeLine("ok")
            out.write("testing user exception...")
            do {
                _ = try prx.amdBadAdd(x: 33, y: 12)
                try test(false)
            } catch is InvalidInputException {
                // Expected
            }
            try test(interceptor.lastOperation == "amdBadAdd")
            try test(interceptor.lastStatus)
            out.writeLine("ok")

            out.write("testing ONE... ")
            interceptor.clear()
            do {
                _ = try prx.amdNotExistAdd(x: 33, y: 12)
                try test(false)
            } catch is ObjectNotExistException {
                // Expected
            }
            try test(interceptor.lastOperation == "amdNotExistAdd")
            try test(interceptor.lastStatus)
            out.writeLine("ok")
        }
    }

    public override func run(args: [String]) throws {
        let properties = try createTestProperties(args)
        properties.setProperty(key: "Ice.Package.Test", value: "test.Ice.interceptor")
        properties.setProperty(key: "Ice.Warn.Dispatch", value: "0")
        let communicator = try initialize(properties)
        defer {
            communicator.destroy()
        }
        communicator.getProperties().setProperty(key: "MyOA.AdapterId", value: "myOA")
        let oa = try communicator.createObjectAdapterWithEndpoints(name: "MyOA2", endpoints: "tcp -h localhost")
        let interceptor = InterceptorI(MyObjectDisp(MyObjectI()))
        var prx = uncheckedCast(prx: try oa.addWithUUID(interceptor), type: MyObjectPrx.self)

        let out = getWriter()
        out.writeLine("Collocation optimization on")
        try runTest(prx: prx, interceptor: interceptor)
        out.writeLine("Now with AMD")
        interceptor.clear()
        try runAmdTest(prx: prx, interceptor: interceptor, out: out)
        try oa.activate() // Only necessary for non-collocation optimized tests

        out.writeLine("Collocation optimization off")
        interceptor.clear()
        prx = uncheckedCast(prx: prx.ice_collocationOptimized(false), type: MyObjectPrx.self)
        try runTest(prx: prx, interceptor: interceptor)

        out.writeLine("Now with AMD")
        interceptor.clear()
        try runAmdTest(prx: prx, interceptor: interceptor, out: out)
    }
}
