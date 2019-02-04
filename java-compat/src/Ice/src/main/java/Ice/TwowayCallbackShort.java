//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package Ice;

public interface TwowayCallbackShort extends TwowayCallback
{
    /**
     * Called when the invocation response is received.
     *
     * @param arg The operation return value.
     **/
    void response(short arg);
}
