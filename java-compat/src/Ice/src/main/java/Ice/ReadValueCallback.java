// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Callback class to inform an application when an instance of a Slice class has been
 * unmarshaled from an input stream.
 *
 * @see InputStream#readValue
 **/
public interface ReadValueCallback
{
    /**
     * The Ice run time calls this method when it has fully unmarshaled the state
     * of a Slice class instance.
     *
     * @param obj The unmarshaled Slice class instance.
     **/
    void valueReady(Ice.Object obj);
}
