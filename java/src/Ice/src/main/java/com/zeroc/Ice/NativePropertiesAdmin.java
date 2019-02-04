//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * Interface implemented by the Property admin facet. It allows users to add and remove
 * callbacks that notify the application when Ice properties are updated at run time.
 **/
public interface NativePropertiesAdmin
{
    /**
     * Adds a new update callback. The callback receives the updated properties each time
     * the properties are updated.
     * @param callback The new callback.
     **/
    void addUpdateCallback(java.util.function.Consumer<java.util.Map<String, String>> callback);

    /**
     * Removes a previously registered update callback.
     * @param callback The callback to remove.
     **/
    void removeUpdateCallback(java.util.function.Consumer<java.util.Map<String, String>> callback);
}
