// Copyright (c) ZeroC, Inc.

/// Implements SliceLoader using the generated code.
public class DefaultSliceLoader: SliceLoader {
    private let classResolverPrefix: [String]

    // We cache successful resolutions. The size of this cache is bounded by the number of Slice classes and exceptions
    // in the program.
    private var typeIdToClassMap: [String: AnyObject.Type] = [:]
    private var mutex = Mutex()

    public init(_ classResolverPrefix: String...) {
        self.classResolverPrefix = classResolverPrefix
    }

    public func newInstance(_ typeId: String) -> AnyObject? {
        var cls: AnyObject.Type?
        mutex.sync {
            cls = typeIdToClassMap[typeId]
        }
        if cls == nil {
            if classResolverPrefix.isEmpty {
                cls = ClassResolver.resolve(typeId: typeId)
            } else {
                for prefix in classResolverPrefix {
                    cls = ClassResolver.resolve(typeId: typeId, prefix: prefix)
                    if cls != nil {
                        break
                    }
                }
            }
            if (cls != nil) {
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
}
