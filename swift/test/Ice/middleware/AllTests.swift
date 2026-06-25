// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

func allTests(_ helper: TestHelper) async throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    let communicator = helper.communicator()
    let output = helper.getWriter()

    try await testMiddlewareExecutionOrder(communicator, output)
    try await testMiddlewareObservesLocatorDispatchError(communicator, output)

    // Verifies the middleware execute in installation order.
    func testMiddlewareExecutionOrder(_ communicator: Communicator, _ output: TextWriter) async throws {
        output.write("testing middleware execution order... ")

        // Arrange
        let oa = try communicator.createObjectAdapterWithEndpoints(
            name: "MyOA", endpoints: "tcp -h 127.0.0.1 -p 0")
        let log = MiddlewareLog()

        let objPrx = try oa.add(servant: MyObjectI(), id: Ice.Identity(name: "test"))
        oa.use { next in
            Middleware(next, "A", log)
        }.use { next in
            Middleware(next, "B", log)
        }.use { next in
            Middleware(next, "C", log)
        }

        // We're actually using colloc so no need to activate oa.

        let p = uncheckedCast(prx: objPrx, type: MyObjectPrx.self)

        // Act
        try await p.ice_ping()

        // Assert
        try await test(log.inLog == ["A", "B", "C"])
        try await test(log.outLog == ["C", "B", "A"])

        output.writeLine("ok")
        oa.destroy()
    }

    // Verifies that a dispatch error from a servant returned by a ServantLocator propagates through the
    // middleware pipeline, instead of being converted to a response inside ServantManager (issue #5555).
    func testMiddlewareObservesLocatorDispatchError(_ communicator: Communicator, _ output: TextWriter)
        async throws
    {
        output.write("testing middleware observes a servant-locator dispatch error... ")

        let oa = try communicator.createObjectAdapterWithEndpoints(
            name: "MyOA2", endpoints: "tcp -h 127.0.0.1 -p 0")
        try oa.addServantLocator(locator: ThrowingServantLocator(), category: "")

        let log = ThrowObservedLog()
        oa.use { next in ErrorObservingMiddleware(next, log) }

        // We're actually using colloc so no need to activate oa.
        let p = try uncheckedCast(prx: oa.createProxy(Ice.Identity(name: "test")), type: MyObjectPrx.self)

        var threw = false
        do {
            _ = try await p.getName()
        } catch {
            threw = true
        }

        // The located servant always throws, so the client must see a dispatch error, and the middleware
        // must have observed it as a throw.
        try test(threw)
        try await test(log.observed)

        output.writeLine("ok")
        oa.destroy()
    }

    final class ThrowingObjectI: MyObject {
        func getName(current: Ice.Current) throws -> String {
            throw Ice.ObjectNotExistException()
        }
    }

    final class ThrowingServantLocator: Ice.ServantLocator {
        func locate(_ curr: Ice.Current) throws -> (returnValue: Ice.Dispatcher?, cookie: AnyObject?) {
            (ThrowingObjectI(), nil)
        }
        func finished(curr: Ice.Current, servant: Ice.Dispatcher, cookie: AnyObject?) throws {}
        func deactivate(_ category: String) {}
    }

    final class ErrorObservingMiddleware: Dispatcher {
        private let next: Dispatcher
        private let log: ThrowObservedLog

        init(_ next: Dispatcher, _ log: ThrowObservedLog) {
            self.next = next
            self.log = log
        }

        func dispatch(_ request: sending IncomingRequest) async throws -> OutgoingResponse {
            do {
                return try await next.dispatch(request)
            } catch {
                await log.observe()
                throw error
            }
        }
    }

    actor ThrowObservedLog {
        var observed = false
        func observe() { observed = true }
    }

    final class Middleware: Dispatcher {
        private let next: Dispatcher
        private let name: String
        private let log: MiddlewareLog

        init(_ next: Dispatcher, _ name: String, _ log: MiddlewareLog) {
            self.next = next
            self.name = name
            self.log = log
        }

        func dispatch(_ request: sending IncomingRequest) async throws -> OutgoingResponse {
            await log.appendInLog(name)
            let response = try await next.dispatch(request)
            await log.appendOutLog(name)
            return response
        }
    }

    actor MiddlewareLog {
        var inLog: [String] = []
        var outLog: [String] = []

        func appendInLog(_ value: String) async {
            inLog.append(value)
        }
        func appendOutLog(_ value: String) async {
            outLog.append(value)
        }
    }

    final class MyObjectI: MyObject {
        func getName(current: Ice.Current) -> String {
            "Foo"
        }
    }
}
