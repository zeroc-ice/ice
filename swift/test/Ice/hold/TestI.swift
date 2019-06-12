//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Dispatch
import Foundation
import Ice
import TestCommon

class HoldI: Hold {
    var _adapter: Ice.ObjectAdapter
    var _helper: TestHelper
    var _last: Int32 = 0
    var _lock = os_unfair_lock()
    var _queue = DispatchQueue(label: "ice.hold.Server")

    init(adapter: Ice.ObjectAdapter, helper: TestHelper) {
        _adapter = adapter
        _helper = helper
    }

    func putOnHold(seconds: Int32, current: Ice.Current) throws {
        if seconds < 0 {
            _adapter.hold()
        } else if seconds == 0 {
            _adapter.hold()
            try _adapter.activate()
        } else {
            _queue.asyncAfter(deadline: .now() + .milliseconds(Int(seconds))) {
                do {
                    try self.putOnHold(seconds: 0, current: current)
                } catch is Ice.ObjectAdapterDeactivatedException {} catch {
                    precondition(false)
                }
            }
        }
    }

    func waitForHold(current: Ice.Current) throws {
        _queue.async {
            do {
                current.adapter!.waitForHold()
                try current.adapter!.activate()
            } catch {
                //
                // This shouldn't occur. The test ensures all the waitForHold timers are
                // finished before shutting down the communicator.
                //
                precondition(false)
            }
        }
    }

    func set(value: Int32, delay: Int32, current _: Ice.Current) throws -> Int32 {
        Thread.sleep(forTimeInterval: Double(delay / 1000))
        return withLock(&_lock) {
            let tmp = _last
            _last = value
            return tmp
        }
    }

    func setOneway(value: Int32, expected: Int32, current _: Ice.Current) throws {
        try withLock(&_lock) {
            try self._helper.test(_last == expected)
            _last = value
        }
    }

    func shutdown(current _: Ice.Current) throws {
        _adapter.hold()
        _adapter.getCommunicator().shutdown()
    }
}
