// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

/**
 * Called when the communicator's properties have been updated.
 * @param PropertyDict A dictionary containing the properties that were added, changed or removed,
 * with a removed property denoted by an entry whose value is an empty string.
 */
typealias PropertiesAdminUpdateCallback = (PropertyDict) -> Void

typealias PropertiesAdminRemoveallback = () -> Void

/**
 * Base protocol for the Properties admin facet.
 */
protocol NativePropertiesAdmin {
    /**
     * Register an update callback that will be invoked when property updates occur.
     * @param cb The callback.
     * @return A callback to remove cb
     */
    func addUpdateCallback(_ cb: @escaping PropertiesAdminUpdateCallback) -> PropertiesAdminRemoveallback
}
