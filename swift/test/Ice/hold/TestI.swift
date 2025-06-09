// Copyright (c) ZeroC, Inc.

import Dispatch
import Foundation
import Ice
import TestCommon

actor HoldI: Hold {
    private let _adapter: Ice.ObjectAdapter
    private let _helper: TestHelper
    private var _last: Int32 = 0
    private let _queue = DispatchQueue(label: "ice.hold.Server")

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
            _queue.asyncAfter(deadline: .now() + .milliseconds(Int(delay))) { [_adapter] in
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
        let adapter = current.adapter
        _queue.async {
            do {
                adapter.waitForHold()
                try adapter.activate()
            } catch {
                // This shouldn't occur. The test ensures all the waitForHold timers are
                // finished before shutting down the communicator.
                preconditionFailure()
            }
        }
    }

    func shutdown(current _: Ice.Current) async throws {
        _adapter.hold()
        _adapter.getCommunicator().shutdown()
    }
}
