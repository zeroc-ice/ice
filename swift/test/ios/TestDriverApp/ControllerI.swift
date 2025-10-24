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

    func waitReady(timeout: Int32, current _: Ice.Current) async throws {
        try await _helper.waitReady(timeout: timeout)
    }

    func waitSuccess(timeout: Int32, current _: Ice.Current) async throws -> Int32 {
        return try await _helper.waitSuccess(timeout: timeout)
    }

    func terminate(current: Ice.Current) throws -> String {
        _helper.shutdown()
        let adapter = current.adapter
        try adapter.remove(current.id)
        return _helper.getOutput()
    }
}

enum TimeoutError: Error {
    case timedOut
}

func awaitTaskWithTimeout<T>(
    _ task: Task<T, Error>,
    timeout: TimeInterval
) async throws -> T {
    return try await withThrowingTaskGroup(of: T.self) { group in
        // Add the original task to the group
        group.addTask {
            return try await task.value
        }

        // Add a timeout task to the group
        group.addTask {
            try await Task.sleep(nanoseconds: UInt64(timeout * Double(NSEC_PER_SEC)))
            throw TimeoutError.timedOut
        }

        // Wait for the first task to complete
        guard let result = try await group.next() else {
            // This case should ideally not be reached if at least one task is added
            throw TimeoutError.timedOut // Or another appropriate error
        }

        // Cancel any remaining tasks (the other task in this case)
        group.cancelAll()

        return result
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

    func createLogDirectory(path: String, current: Ice.Current) throws -> String {
        return LogHelper.createLogDirectory(path).path
    }
}

class ControllerI {
    private let _communicator: Ice.Communicator

    private static let _controller: Mutex<ControllerI?> = Mutex(nil)

    private init(view: ViewController, ipv4: String, ipv6: String) throws {
        let properties = Ice.createProperties()

        properties.setProperty(key: "Ice.LogFile", value: LogHelper.getControllerLogFile().path)
        properties.setProperty(key: "Ice.Trace.Dispatch", value: "1")
        properties.setProperty(key: "Ice.Plugin.IceDiscovery", value: "1")
        properties.setProperty(key: "Ice.ThreadPool.Server.SizeMax", value: "10")
        properties.setProperty(key: "IceDiscovery.DomainId", value: "TestController")
        properties.setProperty(key: "ControllerAdapter.Endpoints", value: "tcp")
        properties.setProperty(key: "ControllerAdapter.AdapterId", value: UUID().uuidString)

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

actor StatusState {
    var value: Int32? = nil

    func set(_ value: Int32) {
        precondition(self.value == nil)
        self.value = value
    }
}

class ControllerHelperI: ControllerHelper, TextWriter, @unchecked Sendable {
    private let _view: ViewController
    private let _args: [String]
    private var _communicator: Ice.Communicator?
    private let _exe: String
    private let _testName: String
    private var _out: String = ""

    private let _readyTask: Task<Void, Never>
    private let _readyContinuation: CheckedContinuation<Void, Never>

    private let _status = StatusState()
    private let _completedTask: Task<Void, Never>
    private let _completedContinuation: CheckedContinuation<Void, Never>

    public init(view: ViewController, testName: String, args: [String], exe: String) {
        _view = view
        _testName = testName
        _args = args
        _communicator = nil
        _exe = exe

        let group = DispatchGroup()

        let readyContinuation: CheckedContinuation<Void, Never>!
        let completedContinuation: CheckedContinuation<Void, Never>!

        group.enter()
        _readyTask = Task {
            await withCheckedContinuation { continuation in
                readyContinuation = continuation
                group.leave()
            }
        }

        group.enter()
        _completedTask = Task {
            await withCheckedContinuation { continuation in
                completedContinuation = continuation
                group.leave()
            }
        }

        group.wait()
        _readyContinuation = readyContinuation
        _completedContinuation = completedContinuation
    }

    public func serverReady() {
        _readyContinuation.resume()
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

    public func completed(status: Int32) async {
        await _status.set(status)
        _completedContinuation.resume()
    }

    public func run() {

        let className = "\(_testName).\(_exe)"

        Task {
            do {
                let testHelper = TestBundle.getTestHelper(name: className)
                testHelper.setControllerHelper(controllerHelper: self)
                testHelper.setWriter(writer: self)
                try await testHelper.run(args: self._args)
                await completed(status: 0)
            } catch {
                self.writeLine("Error: \(error)")
                await completed(status: 1)
            }
        }
    }

    public func shutdown() {
        guard let communicator = _communicator else {
            return
        }
        communicator.shutdown()
    }

    public func waitReady(timeout: Int32) async throws {
        return try await withThrowingTaskGroup(of: Void.self) { group in
            // Add a timeout task to the group
            group.addTask {
                try await Task.sleep(for: .seconds(Int(timeout)))
                throw CommonProcessFailedException(reason: "timed out waiting for the process to succeed")
            }

            group.addTask { await self._readyTask.value }
            group.addTask { await self._completedTask.value }

            // Wait for the FIRST task to complete (winner takes all)
            try await group.next()
            group.cancelAll()

            let status = await self._status.value

            switch status {
                case .some(let status) where status != 0:
                    throw CommonProcessFailedException(reason: self._out)
                default:
                    return
            }
        }
    }

    public func waitSuccess(timeout: Int32) async throws -> Int32 {
        return try await withThrowingTaskGroup(of: Void.self) { group in
            // Add a timeout task to the group
            group.addTask {
                try await Task.sleep(for: .seconds(Int(timeout)))
                throw CommonProcessFailedException(reason: "timed out waiting for the process to succeed")
            }

            group.addTask { await self._completedTask.value }

            // Wait for the FIRST task to complete (winner takes all)
            try await group.next()
            group.cancelAll()

            let status = await self._status.value

            switch status {
                case .none:
                    fatalError("Completed status is nil")
                case .some(let status) where status != 0:
                    throw CommonProcessFailedException(reason: self._out)
                case .some(let status):
                    return status
            }
        }
    }

    public func getOutput() -> String {
        return _out
    }
}

class LogHelper {
    static func createLogDirectory(_ path: String) -> URL {
        // Get the appropriate directory (Documents for iOS)
        guard let documentsDirectory = FileManager.default.urls(for: .cachesDirectory, in: .userDomainMask).first else {
            fatalError("Unable to access documents directory")
        }

        // Create log directory path
        let logDirectory = documentsDirectory.appendingPathComponent(path)

        // Create directory if it doesn't exist
        try? FileManager.default.createDirectory(at: logDirectory, withIntermediateDirectories: true, attributes: nil)

        return logDirectory
    }

    static func getControllerLogFile() -> URL {
        // Create timestamp in MMddyy-HHmm format
        let dateFormatter = DateFormatter()
        dateFormatter.dateFormat = "MMddyy-HHmm"
        let timestamp = dateFormatter.string(from: Date())

        let logDir = createLogDirectory("controller")
        return logDir.appendingPathComponent("controller-\(timestamp).log")
    }
}
