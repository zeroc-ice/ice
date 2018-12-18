// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package com.zeroc.IceInternal;

public class RemoteObserverI
    extends com.zeroc.IceMX.ObserverWithDelegate<com.zeroc.IceMX.RemoteMetrics,
                                                 com.zeroc.Ice.Instrumentation.RemoteObserver>
    implements com.zeroc.Ice.Instrumentation.RemoteObserver
{
    @Override
    public void reply(final int size)
    {
        forEach(new MetricsUpdate<com.zeroc.IceMX.RemoteMetrics>()
                {
                    @Override
                    public void update(com.zeroc.IceMX.RemoteMetrics v)
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
