// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package com.zeroc.IceInternal;

public class CollocatedObserverI
    extends com.zeroc.IceMX.ObserverWithDelegate<com.zeroc.IceMX.CollocatedMetrics,
                                                 com.zeroc.Ice.Instrumentation.CollocatedObserver>
    implements com.zeroc.Ice.Instrumentation.CollocatedObserver
{
    @Override
    public void
    reply(final int size)
    {
        forEach(new MetricsUpdate<com.zeroc.IceMX.CollocatedMetrics>()
                {
                    @Override
                    public void update(com.zeroc.IceMX.CollocatedMetrics v)
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
