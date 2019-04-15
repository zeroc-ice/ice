//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

/**
 * Called when the communicator's properties have been updated.
 * @param PropertyDict A dictionary containing the properties that were added, changed or removed,
 * with a removed property denoted by an entry whose value is an empty string.
 */
public typealias PropertiesAdminUpdateCallback = (PropertyDict) -> Void

public typealias PropertiesAdminRemoveallback = () -> Void

/**
 * Base protocol for the Properties admin facet.
 */
public protocol NativePropertiesAdmin {
    /**
     * Register an update callback that will be invoked when property updates occur.
     * @param cb The callback.
     * @return A callback to remove cb
     */
    func addUpdateCallback(_ cb: @escaping PropertiesAdminUpdateCallback) -> PropertiesAdminRemoveallback
}
