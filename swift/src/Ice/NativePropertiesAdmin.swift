//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

/// Called when the communicator's properties have been updated.
///
/// - parameter: `PropertyDict` A dictionary containing the properties that were added,
///   changed or removed, with a removed property denoted by an entry whose value is an
///   empty string.
public typealias PropertiesAdminUpdateCallback = (PropertyDict) -> Void

/// Closure used to remove the properties update callback.
public typealias PropertiesAdminRemoveCallback = () -> Void

/// Base protocol for the Properties admin facet.
public protocol NativePropertiesAdmin {
    /// Register an update callback that will be invoked when property updates occur.
    ///
    /// - parameter cb: `PropertiesAdminUpdateCallback` - The callback.
    ///
    /// - returns: A closure that can be invoked to remove the callback.
    func addUpdateCallback(_ cb: @escaping PropertiesAdminUpdateCallback) -> PropertiesAdminRemoveCallback
}
