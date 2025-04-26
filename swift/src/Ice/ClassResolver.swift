// Copyright (c) ZeroC, Inc.

import Foundation

// The generated code for Slice classes and exceptions provides an extension for Ice.ClassResolver with a static
// function that returns a SliceTypeResolver.
public class ClassResolver: NSObject {
    private static func resolveImpl(typeId: String, prefix: String) -> AnyObject.Type? {
        return autoreleasepool {
            var selector: Selector?
            if typeId.hasPrefix("::") {
                let flatName = typeId.dropFirst(2).replacingOccurrences(of: "::", with: "_")
                selector = Selector("resolveTypeId\(prefix)_\(flatName)")
            } else {
                // Compact ID
                selector = Selector("resolveTypeId\(prefix)_\(typeId)")
            }

            guard ClassResolver.responds(to: selector) else {
                return nil
            }
            return ClassResolver.perform(selector).takeUnretainedValue() as? AnyObject.Type
        }
    }

    static func resolve(typeId: String, prefix: String) -> AnyObject.Type? {
       resolveImpl(typeId: typeId, prefix: prefix)
    }
}
