// Copyright (c) ZeroC, Inc.

import Foundation

open class SliceTypeResolver: NSObject {
    open func type() -> AnyObject.Type {
        fatalError("Abstract method")
    }
}

//
// The generated code for Slice classes and exceptions provides an extension for
// Ice.ClassResolver with a static function that returns a ValueTypeResolver or
// UserExceptionTypeResolver.
//
public class ClassResolver: NSObject {
    private static func resolveImpl(typeId: String, prefix: String?) -> AnyObject? {
        return autoreleasepool {
            let start = typeId.index(typeId.startIndex, offsetBy: 2)
            let selector = Selector(
                (prefix ?? "") + typeId[start...].replacingOccurrences(of: "::", with: "_"))
            guard ClassResolver.responds(to: selector) else {
                return nil
            }
            return ClassResolver.perform(selector).takeUnretainedValue()
        }
    }

    static func resolve(typeId: String, prefix: String? = nil) -> AnyObject.Type? {
        guard let t = resolveImpl(typeId: typeId, prefix: prefix) as? SliceTypeResolver else {
            return nil
        }
        return t.type()
    }
}

public class TypeIdResolver: NSObject {
    static func resolve(compactId: Int32) -> String? {
        return autoreleasepool {
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
}
