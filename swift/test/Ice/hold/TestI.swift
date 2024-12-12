// Copyright (c) ZeroC, Inc.

import Dispatch
import Foundation
@preconcurrency import Ice
import TestCommon

class HoldI: Hold, @unchecked Sendable {
    var _adapter: Ice.ObjectAdapter
    var _helper: TestHelper
    var _last: Int32 = 0
    var _queue = DispatchQueue(label: "ice.hold.Server")

    init(adapter: Ice.ObjectAdapter, helper: TestHelper) {
        _adapter = adapter
        _helper = helper
    }

    func putOnHold(delay: Int32, current _: Ice.Current) async throws {
        if delay < 0 {
            _adapter.hold()
        } else if delay == 0 {
            _adapter.hold()
            try _adapter.activate()
        } else {
            _queue.asyncAfter(deadline: .now() + .milliseconds(Int(delay))) {  [self] in
                do {
                    _adapter.hold()
                    try _adapter.activate()
                } catch is Ice.ObjectAdapterDeactivatedException {} catch {
                    preconditionFailure()
                }
            }
        }
    }

    func waitForHold(current: Ice.Current) async throws {
        _queue.async {
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

    func shutdown(current _: Ice.Current) async throws {
        _adapter.hold()
        _adapter.getCommunicator().shutdown()
    }
}
