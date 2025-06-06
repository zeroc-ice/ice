// Copyright (c) ZeroC, Inc.

import Combine
import Foundation
import Ice
import TestCommon

actor State {
    var _batchCount: Int32 = 0
    var _batchPublisher: CurrentValueSubject<Int32, Never> = CurrentValueSubject(0)
    var _shutdown: Bool = false
    var _pending: CheckedContinuation<Void, Never>?

    func setContinuation() async {
        return await withCheckedContinuation { continuation in
            if _shutdown {
                continuation.resume(returning: ())
            } else if let pending = _pending {
                pending.resume(returning: ())
            }
            _pending = continuation
        }
    }

    func callContinuation() {
        if _shutdown {
            return
        } else if let pending = _pending {
            pending.resume(returning: ())
            _pending = nil
        }
    }

    func incrementBatchCount() {
        _batchCount += 1
        _batchPublisher.send(_batchCount)
    }

    func getBatchCount() -> Int32 {
        return _batchCount
    }

    func clearBatchCount() {
        _batchCount = 0
        _batchPublisher.send(0)
    }

    func shutdown() {
        _shutdown = true
        if let pending = _pending {
            pending.resume(returning: ())
            _pending = nil
        }
    }

    func waitForBatch(_ count: Int32) async {
        nonisolated(unsafe) var sub: AnyCancellable?
        await withCheckedContinuation { continuation in
            sub = _batchPublisher.sink { batchCount in
                if batchCount == count {
                    continuation.resume()
                } else if batchCount > count {
                    fatalError("Received more batches than expected: \(batchCount) > \(count)")
                }
            }
        }
        sub!.cancel()
    }

    func sleep(ms: Int32) async throws {
        try await Task.sleep(for: .milliseconds(100))
    }
}

final class TestI: TestIntf {
    private let _state = State()
    private let _helper: TestHelper

    init(helper: TestHelper) {
        _helper = helper
    }

    func op(current _: Current) {}

    func opWithPayload(seq _: ByteSeq, current _: Current) {}

    func opWithResult(current _: Current) -> Int32 {
        return 15
    }

    func opWithUE(current _: Current) throws {
        throw TestIntfException()
    }

    func opWithResultAndUE(current _: Current) throws -> Int32 {
        throw TestIntfException()
    }

    func opWithArgs(current _: Current) -> (
        one: Int32,
        two: Int32,
        three: Int32,
        four: Int32,
        five: Int32,
        six: Int32,
        seven: Int32,
        eight: Int32,
        nine: Int32,
        ten: Int32,
        eleven: Int32
    ) {
        return (1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11)
    }

    func startDispatch(current _: Current) async {
        await _state.setContinuation()
    }

    func pingBiDir(reply: PingReplyPrx?, current: Current) async throws {
        let expectSuccess = !current.ctx.keys.contains("ONE")
        do {
            try await reply!.ice_fixed(current.con!).reply()
            if !expectSuccess {
                throw TestIntfException()
            }
        } catch is ObjectNotExistException {
            if expectSuccess {
                throw TestIntfException()
            }
        }
    }

    func opBatch(current _: Current) async {
        await _state.incrementBatchCount()
    }

    func opBatchCount(current _: Current) async -> Int32 {
        return await _state.getBatchCount()
    }

    func waitForBatch(count: Int32, current _: Current) async -> Bool {
        await _state.waitForBatch(count)
        // Check the batch count again after the sink has been cancelled to get the current value.
        // The client only ever sends the expect number of batches so it is impossible that the
        // batch count will be greater than the expected count at this point unless there is a bug.
        let batchCount = await _state.getBatchCount()
        let result = count == batchCount
        await _state.clearBatchCount()
        return result
    }

    func closeConnection(current: Current) {
        let connection = current.con!
        Task {
            do {
                try await connection.close()
            } catch {
                fatalError("Connection.close failed: \(error)")
            }
        }
    }

    func abortConnection(current: Current) {
        current.con!.abort()
    }

    func sleep(ms: Int32, current _: Current) async throws {
        try await _state.sleep(ms: ms)
    }

    func finishDispatch(current _: Current) async {
        await _state.callContinuation()
    }

    func shutdown(current: Current) async {
        await _state.shutdown()
        current.adapter.getCommunicator().shutdown()
    }

    func supportsFunctionalTests(current _: Current) -> Bool {
        return false
    }

    func supportsBackPressureTests(current _: Current) -> Bool {
        return false
    }
}

final class TestII: OuterInnerTestIntf {
    func op(i: Int32, current _: Ice.Current) throws -> (returnValue: Int32, j: Int32) {
        return (i, i)
    }
}

final class TestControllerI: TestIntfController {
    let _adapter: Ice.ObjectAdapter

    init(adapter: Ice.ObjectAdapter) {
        _adapter = adapter
    }

    func holdAdapter(current _: Ice.Current) {
        _adapter.hold()
    }

    func resumeAdapter(current _: Ice.Current) throws {
        try _adapter.activate()
    }
}
