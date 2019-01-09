// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package Ice;

public interface TwowayCallbackInt extends TwowayCallback
{
    /**
     * Called when the invocation response is received.
     *
     * @param arg The operation return value.
     **/
    void response(int arg);
}
