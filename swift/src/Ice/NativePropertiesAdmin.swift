// Copyright (c) ZeroC, Inc.

import IceImpl

/// Closure called when the communicator's properties have been updated.
///
/// - Parameter: `PropertyDict` A dictionary containing the properties that were added,
///   changed, or removed. Removed properties are denoted by an entry whose value is an
///   empty string.
public typealias PropertiesAdminUpdateCallback = (PropertyDict) -> Void

/// Closure used to remove a ``PropertiesAdminUpdateCallback``.
public typealias PropertiesAdminRemoveCallback = () -> Void

/// The default implementation of the "Properties" admin facet.
public struct NativePropertiesAdmin: PropertiesAdmin {
    nonisolated(unsafe) private let handle: ICEPropertiesAdmin

    init(handle: ICEPropertiesAdmin) {
        self.handle = handle
    }

    public func getProperty(key: String, current _: Current) throws -> String {
        return try autoreleasepool {
            try handle.getProperty(key)
        }
    }

    public func getPropertiesForPrefix(prefix: String, current _: Current) throws -> PropertyDict {
        return try autoreleasepool {
            try handle.getPropertiesForPrefix(prefix)
        }
    }

    public func setProperties(newProperties: PropertyDict, current _: Current) throws {
        try autoreleasepool {
            try handle.setProperties(newProperties)
        }
    }

    /// Registers an update callback that will be invoked when a property update occurs.
    ///
    /// - Parameter cb: The callback.
    /// - Returns: A closure that unregisters the callback.
    public func addUpdateCallback(_ cb: @escaping PropertiesAdminUpdateCallback) -> PropertiesAdminRemoveCallback {
        return handle.addUpdateCallback { (props: PropertyDict) in
            cb(props)
        }
    }
}
