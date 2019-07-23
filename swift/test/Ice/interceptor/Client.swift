//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import PromiseKit
import TestCommon

enum InterceptorError: Error {
    case retry
}

class InterceptorI: Disp {
    public private(set) var servantDisp: Disp
    public private(set) var lastOperation: String?
    public private(set) var lastStatus: Bool = false

    init(_ servantDisp: Disp) {
        self.servantDisp = servantDisp
    }

    func dispatch(request: Request, current: Current) throws -> Promise<Ice.OutputStream>? {
        if let context = current.ctx["raiseBeforeDispatch"] {
            if context == "user" {
                throw InvalidInputException()
            } else if context == "notExist" {
                throw ObjectNotExistException()
            }
        }
        lastOperation = current.operation

        if lastOperation == "addWithRetry" || lastOperation == "amdAddWithRetry" {
            for _ in 0 ..< 10 {
                do {
                    if let p = try servantDisp.dispatch(request: request, current: current) {
                        guard let error = p.error,
                            let errorType = error as? InterceptorError,
                            errorType == .retry
                        else {
                            fatalError("Expected an error")
                        }
                    } else {
                        fatalError("Expected an error")
                    }
                } catch InterceptorError.retry {
                    //
                    // Expected, retry
                    //
                }
            }
            current.ctx["retry"] = "no"
        } else if current.ctx["retry"] != nil && current.ctx["retry"] == "yes" {
            _ = try self.servantDisp.dispatch(request: request, current: current)
            _ = try self.servantDisp.dispatch(request: request, current: current)
        }
        // Did not implement add with retry as Swift does not support retrying
        let p = try servantDisp.dispatch(request: request, current: current)
        lastStatus = p != nil

        if let context = current.ctx["raiseAfterDispatch"] {
            if context == "user" {
                throw InvalidInputException()
            } else if context == "notExist" {
                throw ObjectNotExistException()
            }
        }
        return p
    }

    public func clear() {
        lastOperation = nil
        lastStatus = false
    }
}

class MyObjectI: MyObject {
    func add(x: Int32, y: Int32, current _: Current) throws -> Int32 {
        return x + y
    }

    func addWithRetry(x: Int32, y: Int32, current: Current) throws -> Int32 {
        guard let current = current.ctx["retry"], current == "no" else {
            throw InterceptorError.retry
        }
        return x + y
    }

    func badAdd(x _: Int32, y _: Int32, current _: Current) throws -> Int32 {
        throw InvalidInputException()
    }

    func notExistAdd(x _: Int32, y _: Int32, current _: Current) throws -> Int32 {
        throw ObjectNotExistException()
    }

    func amdAddAsync(x: Int32, y: Int32, current _: Current) -> Promise<Int32> {
        return Promise<Int32> { seal in
            DispatchQueue.global().asyncAfter(deadline: .now() + .milliseconds(1000)) {
                seal.fulfill(x + y)
            }
        }
    }

    func amdAddWithRetryAsync(x: Int32, y: Int32, current: Current) -> Promise<Int32> {
        guard let current = current.ctx["retry"], current == "no" else {
            return Promise(error: InterceptorError.retry)
        }

        return Promise<Int32> { seal in
            DispatchQueue.global().asyncAfter(deadline: .now() + .milliseconds(1000)) {
                seal.fulfill(x + y)
            }
        }
    }

    func amdBadAddAsync(x _: Int32, y _: Int32, current _: Current) -> Promise<Int32> {
        return Promise<Int32> { seal in
            DispatchQueue.global().asyncAfter(deadline: .now() + .milliseconds(1000)) {
                seal.reject(InvalidInputException())
            }
        }
    }

    func amdNotExistAddAsync(x _: Int32, y _: Int32, current _: Current) -> Promise<Int32> {
        return Promise<Int32> { seal in
            DispatchQueue.global().asyncAfter(deadline: .now() + .milliseconds(1000)) {
                seal.reject(ObjectNotExistException())
            }
        }
    }
}

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
            out.write("testing retry... ")
            try test(prx.addWithRetry(x: 33, y: 12) == 45)
            try test(interceptor.lastOperation == "addWithRetry")
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

            out.write("testing exceptions raised by the interceptor... ")
            try testInterceptorExceptions(prx)
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

            out.write("testing retry... ")
            try test(prx.amdAddWithRetry(x: 33, y: 12) == 45)
            try test(interceptor.lastOperation == "amdAddWithRetry")
            try test(interceptor.lastStatus)
            var ctx: [String:String] = [:]
            ctx["retry"] = "yes"
            for _ in 0 ..< 10 {
                try test(prx.amdAdd(x: 33, y: 12) == 45)
                try test(interceptor.lastOperation == "amdAdd")
                try test(interceptor.lastStatus)
            }
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

            out.write("testing exceptions raised by the interceptor... ")
            try testInterceptorExceptions(prx)
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

    private func testInterceptorExceptions(_ prx: MyObjectPrx) throws {
        let exceptions: [(point: String, exception: String)] = [
            ("raiseBeforeDispatch", "user"),
            ("raiseBeforeDispatch", "notExist"),
            ("raiseAfterDispatch", "user"),
            ("raiseAfterDispatch", "notExist")
        ]
        for e in exceptions {
            var ctx: Context = [:]
            ctx[e.point] = e.exception
            do {
                try prx.ice_ping(context: ctx)
                try test(false)
            } catch is UnknownUserException {
                try test(e.exception == "user")
            } catch is ObjectNotExistException {
                try test(e.exception == "notExist")
            }
            let batch = prx.ice_batchOneway()
            try batch.ice_ping(context: ctx)
            try batch.ice_ping()
            try batch.ice_flushBatchRequests()

            // Force the last batch request to be dispatched by the server thread using invocation timeouts
            // This is required to prevent threading issue with the test interceptor implementation which
            // isn't thread safe
            try prx.ice_invocationTimeout(10000).ice_ping()
        }
    }
}
