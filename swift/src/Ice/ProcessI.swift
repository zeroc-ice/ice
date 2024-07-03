// Copyright (c) ZeroC, Inc.

import IceImpl

class ProcessI: LocalObject<ICEProcess>, Process {
    func shutdown(current _: Current) {
        handle.shutdown()
    }

    func writeMessage(message: String, fd: Int32, current _: Current) {
        handle.writeMessage(message, fd: fd)
    }
}
