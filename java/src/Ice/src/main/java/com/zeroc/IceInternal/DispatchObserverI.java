//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

public class DispatchObserverI
    extends com.zeroc.Ice.MX.ObserverWithDelegate<com.zeroc.Ice.MX.DispatchMetrics,
                                                 com.zeroc.Ice.Instrumentation.DispatchObserver>
    implements com.zeroc.Ice.Instrumentation.DispatchObserver
{
    @Override
    public void userException()
    {
        forEach(_userException);
        if(_delegate != null)
        {
            _delegate.userException();
        }
    }

    @Override
    public void reply(final int size)
    {
        forEach(new MetricsUpdate<com.zeroc.Ice.MX.DispatchMetrics>()
                {
                    @Override
                    public void update(com.zeroc.Ice.MX.DispatchMetrics v)
                    {
                        v.replySize += size;
                    }
                });
        if(_delegate != null)
        {
            _delegate.reply(size);
        }
    }

    final private MetricsUpdate<com.zeroc.Ice.MX.DispatchMetrics> _userException =
        new MetricsUpdate<com.zeroc.Ice.MX.DispatchMetrics>()
    {
        @Override
        public void update(com.zeroc.Ice.MX.DispatchMetrics v)
        {
            ++v.userException;
        }
    };
}
