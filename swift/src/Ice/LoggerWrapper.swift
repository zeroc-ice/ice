// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import IceObjc

// Wrapps Swift Loggers so they can be callbed by ObjC/C++
class LoggerWrapper: ICELoggerProtocol {
    let handle: Logger

    init(handle: Logger) {
        self.handle = handle
    }

    func print(_ message: String) {
        handle.print(message)
    }

    func trace(category: String, message: String) {
        handle.trace(category: category, message: message)
    }

    func warning(_ message: String) {
        handle.warning(message)
    }

    func error(_ message: String) {
        handle.error(message)
    }

    func getPrefix() -> String {
        return handle.getPrefix()
    }

    func cloneWithPrefix(_ prefix: String) -> Any {
        return handle.cloneWithPrefix(prefix)
    }
}

// Wrapps Ice C++ logger
class ObjcLoggerWrapper: LocalObject<ICELogger>, Logger {
    func print(_ message: String) {
        _handle.print(message)
    }

    func trace(category: String, message: String) {
        _handle.trace(category: category, message: message)
    }

    func warning(_ message: String) {
        _handle.warning(message)
    }

    func error(_ message: String) {
        _handle.error(message)
    }

    func getPrefix() -> String {
        return _handle.getPrefix()
    }

    func cloneWithPrefix(_ prefix: String) -> Logger {
        // swiftlint:disable force_cast
        return ObjcLoggerWrapper(handle: _handle.cloneWithPrefix(prefix) as! ICELogger)
    }
}
