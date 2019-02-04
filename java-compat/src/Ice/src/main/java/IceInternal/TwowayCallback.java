//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public abstract class TwowayCallback extends CallbackBase implements Ice.TwowayCallback
{
    public void sent(boolean sentSynchronously)
    {
    }

    @Override
    public void exception(Ice.SystemException ex)
    {
        exception(new Ice.UnknownException(ex));
    }

    @Override
    public final void _iceSent(Ice.AsyncResult result)
    {
        sent(result.sentSynchronously());
    }

    @Override
    public final boolean _iceHasSentCallback()
    {
        return true;
    }
}
