// Copyright (c) ZeroC, Inc.

import Foundation

/// Implements SliceLoader using the generated code.
public final class DefaultSliceLoader: NSObject, SliceLoader {
    private let classResolverPrefix: String

    // We cache successful resolutions. The size of this cache is bounded by the number of Slice classes and exceptions
    // in the program.
    private var typeIdToClassMap: [String: AnyObject.Type] = [:]
    private var mutex = Mutex()

    /// Creates a DefaultSliceLoader.
    /// - Parameter classResolverPrefix: The prefix to use when resolving Slice classes and exceptions. This prefix
    /// corresponds to the Slice metadata directive swift:class-resolver-prefix.
    public init(_ classResolverPrefix: String = "") {
        self.classResolverPrefix = classResolverPrefix
    }

    public func newInstance(_ typeId: String) -> AnyObject? {
        var cls: AnyObject.Type?
        mutex.sync {
            cls = typeIdToClassMap[typeId]
        }
        if cls == nil {
            cls = Self.resolve(typeId: typeId, prefix: classResolverPrefix)
            if cls != nil {
                mutex.sync {
                    typeIdToClassMap[typeId] = cls
                }
            }
        }
        if let cls = cls {
            if let valueType = cls as? Value.Type {
                return valueType.init()
            } else if let exceptionType = cls as? UserException.Type {
                return exceptionType.init()
            }
            fatalError("The default Slice loader resolved \(typeId) to \(cls), but it's not a Value or UserException.")
        } else {
            return nil
        }
    }

    /// Finds a generated class or exception dynamically, using a resolveTypeId<prefix>_xxx extension method.
    private static func resolve(typeId: String, prefix: String) -> AnyObject.Type? {
        return autoreleasepool {
            // A compact ID does not start with "::".
            let flatName =
                typeId.hasPrefix("::") ? typeId.dropFirst(2).replacingOccurrences(of: "::", with: "_") : typeId

            let selector = Selector("resolveTypeId\(prefix)_\(flatName)")

            guard DefaultSliceLoader.responds(to: selector) else {
                return nil
            }
            return DefaultSliceLoader.perform(selector).takeUnretainedValue() as? AnyObject.Type
        }
    }
}
