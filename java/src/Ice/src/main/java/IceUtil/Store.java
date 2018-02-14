// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceUtil;

/**
 * A base interface for retrieving persistent objects from a backing store.
 **/

public interface Store
{
    /**
     * Instantiate an object, initializing its state from a backing store.
     *
     * @param key The database key for the object to instantiate.
     *
     **/
    Object load(Object key);
}
