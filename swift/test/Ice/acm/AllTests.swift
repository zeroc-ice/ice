//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import Foundation

class LoggerI: Ice.Logger {
    var _name: String
    var _output: TextWriter
    var _started: Bool
    var _messages: [String]
    var _dateFormat: DateFormatter
    var _timeFormat: DateFormatter

    public init(name: String, output: TextWriter) {
        _name = name
        _output = output
        _started = false
        _messages = [String]()

        _dateFormat = DateFormatter()
        _dateFormat.setLocalizedDateFormatFromTemplate("d")

        _timeFormat = DateFormatter()
        _timeFormat.setLocalizedDateFormatFromTemplate("HH:mm:ss:fff")
    }

    public func start() {
        _started = true
        dump()
    }

    public func print(_ msg: String) {
        _messages.append(msg)
        if _started {
            dump()
        }
    }

    public func trace(category: String, message: String) {
        var s = _name
        s += " "
        s += _dateFormat.string(from: Date())
        s += " "
        s += _timeFormat.string(from: Date())
        s += " "
        s += "["
        s += category
        s += "] "
        s += message
        _messages.append(s)
        if _started {
            dump()
        }
    }

    public func warning(_ message: String) {
        var s = _name
        s += " "
        s += _dateFormat.string(from: Date())
        s += " "
        s += _timeFormat.string(from: Date())
        s += " warning : "
        s += message
        _messages.append(s)
        if _started {
            dump()
        }
    }

    public func error(_ message: String) {
        var s = _name
        s += " "
        s += _dateFormat.string(from: Date())
        s += " "
        s += _timeFormat.string(from: Date())
        s += " error : "
        s += message
        _messages.append(s)
        if _started {
            dump()
        }
    }

    public func getPrefix() -> String {
        return ""
    }

    public func cloneWithPrefix(_ prefix: String) -> Ice.Logger {
        return self
    }

    private func dump() {
        for line in _messages {
            _output.writeLine(line)
        }
        _messages = []
    }
}

public class AllTests {
    public static func allTests(helper: TestHelper) throws {
        func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
            try helper.test(value, file: file, line: line)
        }
    }
}
