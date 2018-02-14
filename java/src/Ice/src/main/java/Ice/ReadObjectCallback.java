// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Callback class to inform an application when a Slice class has been unmarshaled
 * from an input stream.
 *
 * @see InputStream#readObject
 **/
public interface ReadObjectCallback
{
    /**
     * The Ice run time calls this method when it has fully unmarshaled the state
     * of a Slice class.
     *
     * @param obj The unmarshaled Slice class.
     **/
    void invoke(Ice.Object obj);
}
