// Copyright (c) ZeroC, Inc.

import Foundation
import Ice
import TestBundle
import TestCommon

class ProcessI: CommonProcess {
    var _helper: ControllerHelperI

    init(helper: ControllerHelperI) {
        _helper = helper
    }

    func waitReady(timeout: Int32, current _: Ice.Current) async throws {
        try _helper.waitReady(timeout: timeout)
    }

    func waitSuccess(timeout: Int32, current _: Ice.Current) async throws -> Int32 {
        return try _helper.waitSuccess(timeout: timeout)
    }

    func terminate(current: Ice.Current) async throws -> String {
        _helper.shutdown()
        let adapter = current.adapter
        try adapter.remove(current.id)
        return _helper.getOutput()
    }
}

class ProcessControllerI: CommonProcessController {
    var _view: ViewController
    var _ipv4: String
    var _ipv6: String

    var _currentTest: String = ""

    //
    // Run each process in its own queue
    //
    var _serverDispatchQueue: DispatchQueue
    var _clientDispatchQueue: DispatchQueue

    init(view: ViewController, ipv4: String, ipv6: String) {
        _view = view
        _ipv4 = ipv4
        _ipv6 = ipv6

        //TODO: remove the queues
        _serverDispatchQueue = DispatchQueue(label: "Server", qos: .background)
        _clientDispatchQueue = DispatchQueue(label: "Client", qos: .background)
    }

    func start(
        testsuite: String,
        exe: String,
        args: CommonStringSeq,
        current: Ice.Current
    ) throws -> CommonProcessPrx? {

        _view.println("starting \(testsuite) \(exe)... ")

        _currentTest = testsuite.replacingOccurrences(of: "/", with: "_")

        if exe == "ServerAMD" {
            _currentTest += "AMD"
        }

        let helper = ControllerHelperI(
            view: _view,
            testName: _currentTest,
            args: args,
            exe: exe,
            queue: (exe == "Server" || exe == "ServerAMD") ? _serverDispatchQueue : _clientDispatchQueue)
        helper.run()
        return try uncheckedCast(
            prx: current.adapter.addWithUUID(
                ProcessI(helper: helper)), type: CommonProcessPrx.self)
    }

    func getHost(
        transportProtocol _: String,
        ipv6: Bool,
        current _: Ice.Current
    ) throws -> String {
        return ipv6 ? _ipv6 : _ipv4
    }
}

class ControllerI {
    var _communicator: Ice.Communicator!
    static var _controller: ControllerI!

    init(view: ViewController, ipv4: String, ipv6: String) throws {
        let properties = Ice.createProperties()
        properties.setProperty(key: "Ice.Plugin.IceDiscovery", value: "1")
        properties.setProperty(key: "Ice.ThreadPool.Server.SizeMax", value: "10")
        properties.setProperty(key: "IceDiscovery.DomainId", value: "TestController")
        properties.setProperty(key: "ControllerAdapter.Endpoints", value: "tcp")
        properties.setProperty(key: "ControllerAdapter.AdapterId", value: UUID().uuidString)
        // properties.setProperty(key: "Ice.Trace.Protocol", value: "2")
        // properties.setProperty(key: "Ice.Trace.Network", value: "3")

        var initData = Ice.InitializationData()
        initData.properties = properties
        _communicator = try Ice.initialize(initData)

        let adapter = try _communicator.createObjectAdapter("ControllerAdapter")
        var ident = Ice.Identity()

        #if targetEnvironment(simulator)
            ident.category = "iPhoneSimulator"
        #else
            ident.category = "iPhoneOS"
        #endif
        ident.name = "com.zeroc.Swift-Test-Controller"

        try adapter.add(
            servant: ProcessControllerI(view: view, ipv4: ipv4, ipv6: ipv6), id: ident)
        try adapter.activate()
    }

    public func destroy() {
        precondition(_communicator != nil)
        _communicator.destroy()
    }

    public class func stopController() {
        if _controller != nil {
            _controller.destroy()
            _controller = nil
        }
    }

    public class func startController(view: ViewController, ipv4: String, ipv6: String) throws {
        _controller = try ControllerI(view: view, ipv4: ipv4, ipv6: ipv6)
    }
}

class ControllerHelperI: ControllerHelper, TextWriter {
    var _view: ViewController
    var _args: [String]
    var _ready: Bool
    var _completed: Bool
    var _status: Int32
    var _out: String
    var _communicator: Ice.Communicator!
    var _semaphore: DispatchSemaphore
    var _exe: String
    var _queue: DispatchQueue
    var _testName: String

    public init(
        view: ViewController, testName: String, args: [String], exe: String, queue: DispatchQueue
    ) {
        _view = view
        _testName = testName
        _args = args
        _ready = false
        _completed = false
        _status = 1
        _out = ""
        _communicator = nil
        _exe = exe
        _semaphore = DispatchSemaphore(value: 0)
        _queue = queue
    }

    public func serverReady() {
        _ready = true
        _semaphore.signal()
    }

    public func communicatorInitialized(communicator: Ice.Communicator) {
        _communicator = communicator
    }

    public func loggerPrefix() -> String {
        return _testName
    }

    public func write(_ msg: String) {
        _out += msg
    }

    public func writeLine(_ msg: String) {
        write("\(msg)\n")
    }

    public func completed(status: Int32) {
        _completed = true
        _status = status
        _semaphore.signal()
    }

    public func run() {

        let className = "\(_testName).\(_exe)"

        Task {
            do {
                let testHelper = TestBundle.getTestHelper(name: className)
                testHelper.setControllerHelper(controllerHelper: self)
                testHelper.setWriter(writer: self)
                try await testHelper.run(args: self._args)
                self.completed(status: 0)
            } catch {
                self.writeLine("Error: \(error)")
                self.completed(status: 1)
            }
        }
    }

    public func shutdown() {
        guard let communicator = _communicator else {
            return
        }
        communicator.shutdown()
    }

    public func waitReady(timeout: Int32) throws {
        var ex: Error?
        do {
            while !_ready, !_completed {
                if _semaphore.wait(timeout: .now() + Double(timeout)) == .timedOut {
                    throw CommonProcessFailedException(
                        reason: "timed out waiting for the process to be ready")
                }
            }

            if _completed, _status != 0 {
                throw CommonProcessFailedException(reason: _out)
            }
        } catch {
            ex = error
        }
        if let ex = ex {
            throw ex
        }
    }

    public func waitSuccess(timeout: Int32) throws -> Int32 {
        var ex: Error?
        do {
            while !_completed {
                if _semaphore.wait(timeout: .now() + Double(timeout)) == .timedOut {
                    throw CommonProcessFailedException(reason: "timed out waiting for the process to succeed")
                }
            }

            if _completed, _status != 0 {
                throw CommonProcessFailedException(reason: _out)
            }
        } catch {
            ex = error
        }

        if let ex = ex {
            throw ex
        }
        return _status
    }

    public func getOutput() -> String {
        return _out
    }
}
