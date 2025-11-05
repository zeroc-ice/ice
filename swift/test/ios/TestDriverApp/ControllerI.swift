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

class ControllerHelperI: ControllerHelper, TextWriter, @unchecked Sendable {
    private let _view: ViewController
    private let _args: [String]
    private var _communicator: Ice.Communicator? = nil
    private let _exe: String
    private let _testName: String
    private var _out: String = ""

    private let _readyStream: AsyncStream<Void>
    private let _readyStreamContinuation: AsyncStream<Void>.Continuation
    private let _completedStream: AsyncStream<Void>
    private let _completedStreamContinuation: AsyncStream<Void>.Continuation

    private struct State {
        var isReady = false
        var completedStatus: Int32?
    }

    private let _state = Mutex(State())

    public init(view: ViewController, testName: String, args: [String], exe: String) {
        _view = view
        _testName = testName
        _args = args
        _exe = exe

        (_readyStream, _readyStreamContinuation) = AsyncStream<Void>.makeStream()
        (_completedStream, _completedStreamContinuation) = AsyncStream<Void>.makeStream()
    }

    public func serverReady() {
        _state.withLock {
            precondition(!$0.isReady, "serverReady called multiple times")
            $0.isReady = true
        }
        _readyStreamContinuation.finish()
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
        _state.withLock {
            precondition($0.completedStatus == nil, "completed status already set")
            $0.completedStatus = status
        }
        _completedStreamContinuation.finish()
    }

    public func run() {

        let className = "\(_testName).\(_exe)"

        Task {
            do {
                let testHelper = TestBundle.getTestHelper(name: className)
                testHelper.setControllerHelper(controllerHelper: self)
                testHelper.setWriter(writer: self)
                try await testHelper.run(args: self._args)
                completed(status: 0)
            } catch {
                self.writeLine("Error: \(error)")
                completed(status: 1)
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
        enum ReadyResult {
            case completed(Int32)
            case ready
            case timeout
        }

        let result = await withTaskGroup(of: ReadyResult.self) { group in
            group.addTask {
                try? await Task.sleep(for: .seconds(Int(timeout)))
                return .timeout
            }

            group.addTask {
                for await _ in self._readyStream {
                    break
                }
                return .ready
            }

            group.addTask {
                // The task cancellation from another thread that's also reading the stream
                // can cause this task stream's iterator to return. We just loop until
                // we get the completed status or the task is cancelled.
                var completedStatus: Int32?
                while completedStatus == nil && !Task.isCancelled {
                    var iterator = self._completedStream.makeAsyncIterator()
                    _ = await iterator.next()
                    completedStatus = self._state.withLock { $0.completedStatus }
                }

                if Task.isCancelled {
                    return .timeout
                }
                return .completed(completedStatus!)
            }

            // Get the first result
            let result = await group.next()!
            group.cancelAll()

            return result
        }

        switch result {
        case .timeout:
            throw CommonProcessFailedException(reason: "timed out waiting for the process to be ready")
        case .completed(let status) where status != 0:
            throw CommonProcessFailedException(reason: self._out)
        case .ready, .completed(_):
            return
        }
    }

    public func waitSuccess(timeout: Int32) async throws -> Int32 {

        enum WaitResult {
            case completed(Int32)
            case timeout
        }

        let result = await withTaskGroup(of: WaitResult.self) { group in
            group.addTask {
                try? await Task.sleep(for: .seconds(Int(timeout)))
                return .timeout
            }

            group.addTask {
                // The task cancellation from another thread that's also reading the stream
                // can cause this task stream's iterator to return. We just loop until
                // we get the completed status or the task is cancelled.
                var completedStatus: Int32?
                while completedStatus == nil && !Task.isCancelled {
                    var iterator = self._completedStream.makeAsyncIterator()
                    _ = await iterator.next()
                    completedStatus = self._state.withLock { $0.completedStatus }
                }

                if Task.isCancelled {
                    return .timeout
                }
                return .completed(completedStatus!)
            }

            // Get the first result
            let result = await group.next()!
            group.cancelAll()

            return result
        }

        switch result {
        case .completed(let status):
            return status
        case .timeout:
            throw CommonProcessFailedException(reason: "timed out waiting for the process to succeed")
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
