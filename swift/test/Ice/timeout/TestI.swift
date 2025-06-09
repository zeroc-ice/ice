// Copyright (c) ZeroC, Inc.

import Ice

final class TimeoutI: Timeout {
    func op(current _: Current) async throws {}

    func sendData(seq _: ByteSeq, current _: Current) {}

    func sleep(to: Int32, current _: Current) async throws {
        try await Task.sleep(for: .milliseconds(Int(to)))
    }
}

final class ControllerI: Controller {
    let _adapter: Ice.ObjectAdapter

    init(_ adapter: Ice.ObjectAdapter) {
        _adapter = adapter
    }

    func holdAdapter(to: Int32, current: Ice.Current) async throws {
        _adapter.hold()
        if to >= 0 {
            let adapter = _adapter
            Task {
                do {
                    adapter.waitForHold()
                    try await Task.sleep(for: .milliseconds(Int(to)))
                    try adapter.activate()
                } catch {
                    fatalError("unexpected error: \(error)")
                }

            }
        }
    }

    func resumeAdapter(current _: Ice.Current) throws {
        try _adapter.activate()
    }

    func shutdown(current: Ice.Current) {
        current.adapter.getCommunicator().shutdown()
    }
}
