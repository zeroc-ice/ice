//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import IceObjc

class ProcessI: LocalObject<ICEProcess>, Process {
    func shutdown(current _: Current) throws {
        try autoreleasepool {
            try _handle.shutdown()
        }
    }

    func writeMessage(message: Swift.String, fd: Swift.Int32, current _: Current) throws {
        try autoreleasepool {
            try _handle.writeMessage(message, fd: fd)
        }
    }
}
