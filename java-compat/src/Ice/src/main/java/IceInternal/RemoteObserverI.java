// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class RemoteObserverI
    extends IceMX.ObserverWithDelegate<IceMX.RemoteMetrics, Ice.Instrumentation.RemoteObserver>
    implements Ice.Instrumentation.RemoteObserver
{
    @Override
    public void
    reply(final int size)
    {
        forEach(new MetricsUpdate<IceMX.RemoteMetrics>()
                {
                    @Override
                    public void
                    update(IceMX.RemoteMetrics v)
                    {
                        v.replySize += size;
                    }
                });
        if(_delegate != null)
        {
            _delegate.reply(size);
        }
    }
}
