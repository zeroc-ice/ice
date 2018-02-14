// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze;

class EvictorElement
{
    //
    // Clean object; can become modified or destroyed
    //
    static final byte clean = 0;

    //
    // New object; can become clean, dead or destroyed
    //
    static final byte created = 1;

    //
    // Modified object; can become clean or destroyed
    //
    static final byte modified = 2;

    //
    // Being saved. Can become dead or created
    //
    static final byte destroyed = 3;

    //
    // Exists only in the Evictor; for example the object was created
    // and later destroyed (without a save in between), or it was
    // destroyed on disk but is still in use. Can become created.
    //
    static final byte dead = 4;

    EvictorElement(Ice.Identity identity, ObjectStore store)
    {
        this.identity = identity;
        this.store = store;
    }

    final ObjectStore store;
    final Ice.Identity identity;

    //
    // Protected by EvictorI
    //
    java.util.Iterator<EvictorElement> evictPosition = null;
    int usageCount = -1;
    int keepCount = 0;
    boolean stale = false;

    //
    // Protected by this
    //
    ObjectRecord rec = null;
    byte status = clean;
}
