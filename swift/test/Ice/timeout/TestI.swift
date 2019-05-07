//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
import Ice
import Foundation

class TimeoutI: Timeout {
    func op(current: Current) throws {}

    func sendData(seq: ByteSeq, current: Current) throws {}

    func sleep(to: Int32, current: Current) throws {
        Thread.sleep(forTimeInterval: TimeInterval(to) / 1000)
    }
}

class ControllerI: Controller {
    var _adapter: Ice.ObjectAdapter

    init(_ adapter: Ice.ObjectAdapter) {
        _adapter = adapter
    }

    func holdAdapter(to: Int32, current: Ice.Current) throws {
        _adapter.hold()
        if to >= 0 {
            let queue = try current.adapter!.getDispatchQueue()
            queue.async {
                self._adapter.waitForHold()
                queue.asyncAfter(deadline: .now() + .milliseconds(Int(to))) {
                    do {
                        try self._adapter.activate()
                    } catch {
                        precondition(false)
                    }
                }
            }
        }
    }

    func resumeAdapter(current: Ice.Current) throws {
        try _adapter.activate()
    }

    func shutdown(current: Ice.Current) {
        current.adapter!.getCommunicator().shutdown()
    }
}
