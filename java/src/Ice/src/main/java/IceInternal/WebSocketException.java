// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final class WebSocketException extends java.lang.RuntimeException
{
    public WebSocketException(String reason)
    {
        super(reason);
    }

    public WebSocketException(String reason, Throwable cause)
    {
        super(reason, cause);
    }

    public WebSocketException(Throwable cause)
    {
        super(cause);
    }
}
