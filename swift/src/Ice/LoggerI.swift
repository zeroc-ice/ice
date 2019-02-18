// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import os
import IceObjc

class LoggerI: Logger {
    let log: OSLog
    let prefix: String
    let subsystem: String

    init(prefix: String = "") {
        self.subsystem = prefix.isEmpty ? "com.zeroc.ice":  "com.zeroc.ice.\(prefix)"
        self.log = OSLog(subsystem: self.subsystem, category: "")
        self.prefix = prefix
    }

    func print(message: String) {
        os_log("%{public}s", log: self.log, type: .default, message)
    }

    func trace(category: String, message: String) {
        let tLog = OSLog(subsystem: subsystem, category: category)
        os_log("%{public}s", log: tLog, type: .info, message)
    }

    func warning(message: String) {
        os_log("%{public}s", log: self.log, type: .error, message)
    }

    func error(message: String) {
        os_log("%{public}s", log: self.log, type: .fault, message)
    }

    func getPrefix() -> String {
        return prefix
    }

    func cloneWithPrefix(prefix: String) -> Logger {
        return LoggerI(prefix: prefix)
    }
}

// This extension adds an additional clone so that LoggerI cann be used with ObjC's ICELogger
// without needing to use LoggerWrapper
extension LoggerI: ICELoggerProtocol {
    func cloneWithPrefix(prefix: String) -> Any {
        return LoggerI(prefix: prefix)
    }
}
