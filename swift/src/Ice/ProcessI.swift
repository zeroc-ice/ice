// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
