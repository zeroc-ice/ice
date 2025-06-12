// Copyright (c) ZeroC, Inc.

import Foundation
import Ice
import Synchronization
import TestBundle
import TestCommon

struct ProcessI: CommonProcess {
    private let _helper: ControllerHelperI

    init(helper: ControllerHelperI) {
        _helper = helper
    }

    func waitReady(timeout: Int32, current _: Ice.Current) throws {
        try _helper.waitReady(timeout: timeout)
    }

    func waitSuccess(timeout: Int32, current _: Ice.Current) throws -> Int32 {
        return try _helper.waitSuccess(timeout: timeout)
    }

    func terminate(current: Ice.Current) throws -> String {
        _helper.shutdown()
        let adapter = current.adapter
        try adapter.remove(current.id)
        return _helper.getOutput()
    }
}

final class ProcessControllerI: CommonProcessController {
    private let _view: ViewController
    private let _ipv4: String
    private let _ipv6: String

    init(view: ViewController, ipv4: String, ipv6: String) {
        _view = view
        _ipv4 = ipv4
        _ipv6 = ipv6
    }

    @MainActor
    func start(
        testsuite: String,
        exe: String,
        args: CommonStringSeq,
        current: Ice.Current
    ) throws -> CommonProcessPrx? {

        _view.println("starting \(testsuite) \(exe)... ")

        var currentTest = testsuite.replacingOccurrences(of: "/", with: "_")

        if exe == "ServerAMD" {
            currentTest += "AMD"
        }

        let helper = ControllerHelperI(view: _view, testName: currentTest, args: args, exe: exe)

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
    private let _communicator: Ice.Communicator

    private static let _controller: Mutex<ControllerI?> = Mutex(nil)

    private init(view: ViewController, ipv4: String, ipv6: String) throws {
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

    private func destroy() {
        _communicator.destroy()
    }

    public class func startController(view: ViewController, ipv4: String, ipv6: String) throws {
        try _controller.withLock {
            $0 = try ControllerI(view: view, ipv4: ipv4, ipv6: ipv6)
        }

    }

    public class func stopController() {
        _controller.withLock {
            $0?.destroy()
            $0 = nil
        }
    }
}

class ControllerHelperI: ControllerHelper, TextWriter, @unchecked Sendable {
    private let _view: ViewController
    private let _args: [String]
    private var _ready: Bool
    private var _completed: Bool
    private var _status: Int32
    private var _out: String
    private var _communicator: Ice.Communicator!
    private let _semaphore: DispatchSemaphore
    private let _exe: String
    private let _testName: String

    public init(view: ViewController, testName: String, args: [String], exe: String) {
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
