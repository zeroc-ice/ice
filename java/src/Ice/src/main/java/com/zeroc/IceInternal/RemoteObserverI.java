// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
