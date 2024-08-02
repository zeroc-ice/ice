// Copyright (c) ZeroC, Inc.

import Dispatch
import Foundation
import Ice
import TestCommon

class HoldI: Hold {
    var _adapter: Ice.ObjectAdapter
    var _helper: TestHelper
    var _last: Int32 = 0
    var _lock = os_unfair_lock()

    init(adapter: Ice.ObjectAdapter, helper: TestHelper) {
        _adapter = adapter
        _helper = helper
    }

    func putOnHold(seconds: Int32, current: Ice.Current) async throws {
        if seconds < 0 {
            _adapter.hold()
        } else if seconds == 0 {
            _adapter.hold()
            try _adapter.activate()
        } else {
            Task {
                try await Task.sleep(for: .seconds(Int(seconds)))
                do {
                    try await self.putOnHold(seconds: 0, current: current)
                } catch is Ice.ObjectAdapterDeactivatedException {} catch {
                    preconditionFailure()
                }
            }
        }
    }

    func waitForHold(current: Ice.Current) async throws {
        Task {
            do {
                current.adapter.waitForHold()
                try current.adapter.activate()
            } catch {
                //
                // This shouldn't occur. The test ensures all the waitForHold timers are
                // finished before shutting down the communicator.
                //
                preconditionFailure()
            }
        }
    }

    func set(value: Int32, delay: Int32, current _: Ice.Current) async throws -> Int32 {
        try await Task.sleep(for: .milliseconds(Int(delay)))
        return withLock(&_lock) {
            let tmp = _last
            _last = value
            return tmp
        }
    }

    func setOneway(value: Int32, expected: Int32, current _: Ice.Current) async throws {
        try withLock(&_lock) {
            try self._helper.test(_last == expected)
            _last = value
        }
    }

    func shutdown(current _: Ice.Current) async throws {
        _adapter.hold()
        _adapter.getCommunicator().shutdown()
    }
}
