// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public interface TwowayCallbackVoidUE extends TwowayCallback
{
    /**
     * Called when the invocation response is received.
     **/
    void response();

    /**
     * Called when the invocation raises an user exception.
     *
     * @param ex The user exception raised by the operation.
     **/
    void exception(Ice.UserException ex);
}
