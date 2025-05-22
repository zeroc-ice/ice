// Copyright (c) ZeroC, Inc.

import IceImpl

class ProcessI: Process {
    private let handle: ICEProcess

    func shutdown(current _: Current) {
        handle.shutdown()
    }

    func writeMessage(message: String, fd: Int32, current _: Current) {
        handle.writeMessage(message, fd: fd)
    }

    init(handle: ICEProcess) {
        self.handle = handle
    }
}
