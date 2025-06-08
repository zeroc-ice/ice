// Copyright (c) ZeroC, Inc.

import Foundation
import Ice

public protocol TextWriter {
    func write(_ data: String)
    func writeLine(_ data: String)
}

public protocol ControllerHelper {
    func loggerPrefix() -> String
    func serverReady()
    func communicatorInitialized(communicator: Ice.Communicator)
}

class StdoutWriter: TextWriter {
    public func write(_ data: String) {
        fputs(data, stdout)
        fflush(stdout)
    }

    public func writeLine(_ data: String) {
        print(data)
        fflush(stdout)
    }
}

public enum TestFailed: Error {
    case testFailed
}

public protocol TestHelper: AnyObject, Sendable {
    init()
    func run(args: [String]) async throws
    func getTestEndpoint(num: Int32, prot: String) -> String
    func getTestEndpoint(properties: Ice.Properties, num: Int32, prot: String) -> String
    func getTestHost() -> String
    func getTestHost(properties: Ice.Properties) -> String
    func getTestProtocol() -> String
    func getTestProtocol(properties: Ice.Properties) -> String
    func getTestPort(num: Int32) -> Int32
    func getTestPort(properties: Ice.Properties, num: Int32) -> Int32
    func createTestProperties(_ args: [String]) throws -> Ice.Properties
    func createTestProperties(_ args: inout [String]) throws -> Ice.Properties
    func initialize(_ args: [String]) throws -> Ice.Communicator
    func initialize(_ properties: Ice.Properties) throws -> Ice.Communicator
    func initialize(_ initData: Ice.InitializationData) throws -> Ice.Communicator

    func test(_ value: Bool, file: String, line: Int) throws

    func getWriter() -> TextWriter
    func setWriter(writer: TextWriter)

    func communicator() -> Ice.Communicator
    func setControllerHelper(controllerHelper: ControllerHelper)
}

extension TestHelper {
    public func getTestEndpoint(num: Int32 = 0, prot: String = "") -> String {
        return getTestEndpoint(
            properties: communicator().getProperties(),
            num: num,
            prot: prot)
    }

    public func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        if !value {
            let writer = getWriter()
            writer.writeLine("Test failed File: \(file): Line: \(line)")
            throw TestFailed.testFailed
        }
    }
}

open class TestHelperI: TestHelper, @unchecked Sendable {
    private var _controllerHelper: ControllerHelper!
    private var _communicator: Ice.Communicator!
    private var _writer: TextWriter!

    public required init() {}

    open func run(args _: [String]) async throws {
        print("Subclass has not implemented abstract method `run`!")
        abort()
    }

    public func getTestEndpoint(properties: Ice.Properties, num: Int32 = 0, prot: String = "")
        -> String
    {
        var s = ""
        s +=
            (prot == "")
            ? properties.getIceProperty("Ice.Default.Protocol") : prot
        s += " -p "
        let port =
            try! properties.getPropertyAsIntWithDefault(key: "Test.BasePort", value: 12010) + num
        s += String(port)
        return s
    }

    public func getTestHost() -> String {
        return getTestHost(properties: _communicator!.getProperties())
    }

    public func getTestHost(properties: Ice.Properties) -> String {
        return properties.getPropertyWithDefault(key: "Ice.Default.Host", value: "127.0.0.1")
    }

    public func getTestProtocol() -> String {
        return getTestProtocol(properties: _communicator!.getProperties())
    }

    public func getTestProtocol(properties: Ice.Properties) -> String {
        return properties.getPropertyWithDefault(key: "Ice.Default.Protocol", value: "tcp")
    }

    public func getTestPort(num: Int32) -> Int32 {
        return getTestPort(properties: _communicator.getProperties(), num: num)
    }

    public func getTestPort(properties: Ice.Properties, num: Int32) -> Int32 {
        return try! properties.getPropertyAsIntWithDefault(key: "Test.BasePort", value: 12010) + num
    }

    public func createTestProperties(_ args: [String]) throws -> Ice.Properties {
        var remainingArgs = args
        let properties = try Ice.createProperties(&remainingArgs)
        remainingArgs = try properties.parseCommandLineOptions(
            prefix: "Test", options: remainingArgs)
        return properties
    }

    public func createTestProperties(_ args: inout [String]) throws -> Ice.Properties {
        let properties = try Ice.createProperties(&args)
        args = try properties.parseCommandLineOptions(prefix: "Test", options: args)
        return properties
    }

    public func initialize(_ args: [String]) throws -> Ice.Communicator {
        var initData = Ice.InitializationData()
        let props = try createTestProperties(args)
        initData.properties = props
        let communicator = try initialize(initData)
        return communicator
    }

    public func initialize(_ properties: Ice.Properties) throws -> Ice.Communicator {
        var initData = Ice.InitializationData()
        initData.properties = properties
        return try initialize(initData)
    }

    public func initialize(_ initData: Ice.InitializationData) throws -> Ice.Communicator {
        let communicator = try Ice.initialize(initData)
        if _communicator == nil {
            _communicator = communicator
        }
        if _controllerHelper != nil {
            _controllerHelper.communicatorInitialized(communicator: _communicator)
        }
        return communicator
    }

    public func getWriter() -> TextWriter {
        if _writer == nil {
            _writer = StdoutWriter()
        }
        return _writer
    }

    public func setWriter(writer: TextWriter) {
        _writer = writer
    }

    public func communicator() -> Communicator {
        return _communicator
    }

    public func serverReady() {
        if _controllerHelper != nil {
            _controllerHelper.serverReady()
        }
    }

    public func setControllerHelper(controllerHelper: ControllerHelper) {
        precondition(_controllerHelper == nil)
        _controllerHelper = controllerHelper
    }
}
