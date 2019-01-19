//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package Ice;

public interface TwowayCallbackLong extends TwowayCallback
{
    /**
     * Called when the invocation response is received.
     *
     * @param arg The operation return value.
     **/
    void response(long arg);
}
