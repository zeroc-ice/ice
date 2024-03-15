//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

public class CollocatedObserverI
    extends com.zeroc.Ice.MX.ObserverWithDelegate<com.zeroc.Ice.MX.CollocatedMetrics,
                                                 com.zeroc.Ice.Instrumentation.CollocatedObserver>
    implements com.zeroc.Ice.Instrumentation.CollocatedObserver
{
    @Override
    public void
    reply(final int size)
    {
        forEach(new MetricsUpdate<com.zeroc.Ice.MX.CollocatedMetrics>()
                {
                    @Override
                    public void update(com.zeroc.Ice.MX.CollocatedMetrics v)
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
