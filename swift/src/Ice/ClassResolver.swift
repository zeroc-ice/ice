//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation

open class ValueTypeResolver: NSObject {
    open func type() -> Value.Type {
        precondition(false, "Abstract method")
    }
}

open class UserExceptionTypeResolver: NSObject {
    open func type() -> UserException.Type {
        precondition(false, "Abstract method")
    }
}

//
// The generated code for Slice classes and exceptions provides an extension for
// Ice.ClassResolver with a static function that returns a ValueTypeResolver or
// UserExceptionTypeResolver.
//
public class ClassResolver: NSObject {
    private static func resolveImpl(typeId: String, prefix: String?) -> AnyObject? {
        let start = typeId.index(typeId.startIndex, offsetBy: 2)
        let selector = Selector((prefix ?? "") + typeId[start...].replacingOccurrences(of: "::", with: "_"))
        guard ClassResolver.responds(to: selector) else {
            return nil
        }
        return ClassResolver.perform(selector).takeUnretainedValue()
    }

    static func resolve(typeId: String, prefix: String? = nil) -> Value.Type? {
        guard let t = resolveImpl(typeId: typeId, prefix: prefix) as? ValueTypeResolver else {
            return nil
        }
        return t.type()
    }

    static func resolve(typeId: String, prefix: String? = nil) -> UserException.Type? {
        guard let t = resolveImpl(typeId: typeId, prefix: prefix) as? UserExceptionTypeResolver else {
            return nil
        }
        return t.type()
    }
}

public class TypeIdResolver: NSObject {
    static func resolve(compactId: Int32) -> String? {
        let selector = Selector("TypeId_\(compactId)")

        guard TypeIdResolver.responds(to: selector) else {
            return nil
        }

        let val = TypeIdResolver.perform(selector).takeUnretainedValue()

        guard let typeId = val as? String else {
            preconditionFailure("unexpected value type")
        }
        return typeId
    }
}
