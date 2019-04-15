//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import IceObjc
import os

class LoggerI: Logger {
    let log: OSLog
    let prefix: String
    let subsystem: String

    init(prefix: String = "") {
        subsystem = prefix.isEmpty ? "com.zeroc.ice" : "com.zeroc.ice.\(prefix)"
        log = OSLog(subsystem: subsystem, category: "")
        self.prefix = prefix
    }

    func print(_ message: String) {
        os_log("%{public}s", log: log, type: .default, message)
    }

    func trace(category: String, message: String) {
        let tLog = OSLog(subsystem: subsystem, category: category)
        os_log("%{public}s", log: tLog, type: .info, message)
    }

    func warning(_ message: String) {
        os_log("%{public}s", log: log, type: .error, message)
    }

    func error(_ message: String) {
        os_log("%{public}s", log: log, type: .fault, message)
    }

    func getPrefix() -> String {
        return prefix
    }

    func cloneWithPrefix(_ prefix: String) -> Logger {
        return LoggerI(prefix: prefix)
    }
}

// This extension adds an additional clone so that LoggerI cann be used with ObjC's ICELogger
// without needing to use LoggerWrapper
extension LoggerI: ICELoggerProtocol {
    func cloneWithPrefix(_ prefix: String) -> Any {
        return LoggerI(prefix: prefix)
    }
}
