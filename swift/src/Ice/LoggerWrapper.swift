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

    let impl: Logger

    init(impl: Logger) {
        self.impl = impl
    }

    func print(message: String) {
        impl.print(message: message)
    }

    func trace(category: String, message: String) {
        impl.trace(category: category, message: message)
    }

    func warning(message: String) {
        impl.warning(message: message)
    }

    func error(message: String) {
        impl.error(message: message)
    }

    func getPrefix() -> String {
        return impl.getPrefix()
    }

    func cloneWithPrefix(prefix: String) -> Any {
        return impl.cloneWithPrefix(prefix:prefix)
    }
}

// Wrapps Ice C++ logger
class ObjcLoggerWrapper: LocalObject<ICELogger>, Logger {
    func print(message: String) {
        _handle.print(message: message)
    }

    func trace(category: String, message: String) {
        _handle.trace(category: category, message: message)
    }

    func warning(message: String) {
        _handle.warning(message: message)
    }

    func error(message: String) {
        _handle.error(message: message)
    }

    func getPrefix() -> String {
        return _handle.getPrefix()
    }

    func cloneWithPrefix(prefix: String) -> Logger {
        return ObjcLoggerWrapper(handle: _handle.cloneWithPrefix(prefix: prefix) as! ICELogger)
    }
}
