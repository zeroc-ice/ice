//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
