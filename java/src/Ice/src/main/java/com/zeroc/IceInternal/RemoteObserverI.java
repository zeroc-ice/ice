//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

public class RemoteObserverI
    extends com.zeroc.Ice.MX.ObserverWithDelegate<com.zeroc.Ice.MX.RemoteMetrics,
                                                 com.zeroc.Ice.Instrumentation.RemoteObserver>
    implements com.zeroc.Ice.Instrumentation.RemoteObserver
{
    @Override
    public void reply(final int size)
    {
        forEach(new MetricsUpdate<com.zeroc.Ice.MX.RemoteMetrics>()
                {
                    @Override
                    public void update(com.zeroc.Ice.MX.RemoteMetrics v)
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
