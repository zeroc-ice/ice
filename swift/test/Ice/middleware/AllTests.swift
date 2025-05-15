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

    // Verifies the middleware execute in installation order.
    func testMiddlewareExecutionOrder(_ communicator: Communicator, _ output: TextWriter) async throws {
        output.write("testing middleware execution order... ")

        // Arrange
        let oa = try communicator.createObjectAdapterWithEndpoints(
            name: "MyOA", endpoints: "tcp -h 127.0.0.1 -p 0")
        let log = MiddlewareLog()

        let objPrx = try oa.add(servant: MyObjectDisp(MyObjectI()), id: Ice.Identity(name: "test"))
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
        try test(log.inLog == ["A", "B", "C"])
        try test(log.outLog == ["C", "B", "A"])

        output.writeLine("ok")
        oa.destroy()
    }

    final class Middleware: Dispatcher {
        private let next: Dispatcher
        private let name: String
        private var log: MiddlewareLog

        func dispatch(_ request: IncomingRequest) async throws -> OutgoingResponse {
            log.inLog.append(name)
            let response = try await next.dispatch(request)
            log.outLog.append(name)
            return response
        }

        init(_ next: Dispatcher, _ name: String, _ log: MiddlewareLog) {
            self.next = next
            self.name = name
            self.log = log
        }
    }

    final class MiddlewareLog {
        var inLog: [String] = []
        var outLog: [String] = []
    }

    final class MyObjectI: MyObject {
        func getName(current: Ice.Current) async throws -> String {
            "Foo"
        }
    }
}
