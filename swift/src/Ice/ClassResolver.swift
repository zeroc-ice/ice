// Copyright (c) ZeroC, Inc.

import Foundation

// The generated code for Slice classes and exceptions provides an extension for Ice.ClassResolver with a static
// function that returns a SliceTypeResolver.
public class ClassResolver: NSObject {
    private static func resolveImpl(typeId: String, prefix: String?) -> AnyObject.Type? {
        return autoreleasepool {
            var selector: Selector?
            if typeId.hasPrefix("::") {
                let start = typeId.index(typeId.startIndex, offsetBy: 2)
                selector = Selector(
                    (prefix ?? "") + typeId[start...].replacingOccurrences(of: "::", with: "_"))
            } else {
                // Compact ID
                selector = Selector((prefix ?? "") + "TypeId_" + typeId)
            }

            guard ClassResolver.responds(to: selector) else {
                return nil
            }
            return ClassResolver.perform(selector).takeUnretainedValue() as? AnyObject.Type
        }
    }

    static func resolve(typeId: String, prefix: String? = nil) -> AnyObject.Type? {
       resolveImpl(typeId: typeId, prefix: prefix)
    }
}
