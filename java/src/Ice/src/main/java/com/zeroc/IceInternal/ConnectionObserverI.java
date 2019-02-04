//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

public class ConnectionObserverI
    extends com.zeroc.IceMX.ObserverWithDelegate<com.zeroc.IceMX.ConnectionMetrics,
                                                 com.zeroc.Ice.Instrumentation.ConnectionObserver>
    implements com.zeroc.Ice.Instrumentation.ConnectionObserver
{
    @Override
    public void sentBytes(final int num)
    {
        _sentBytes = num;
        forEach(_sentBytesUpdate);
        if(_delegate != null)
        {
            _delegate.sentBytes(num);
        }
    }

    @Override
    public void receivedBytes(int num)
    {
        _receivedBytes = num;
        forEach(_receivedBytesUpdate);
        if(_delegate != null)
        {
            _delegate.receivedBytes(num);
        }
    }

    private MetricsUpdate<com.zeroc.IceMX.ConnectionMetrics> _sentBytesUpdate =
        new MetricsUpdate<com.zeroc.IceMX.ConnectionMetrics>()
        {
            @Override
            public void update(com.zeroc.IceMX.ConnectionMetrics v)
            {
                v.sentBytes += _sentBytes;
            }
        };

    private MetricsUpdate<com.zeroc.IceMX.ConnectionMetrics> _receivedBytesUpdate =
        new MetricsUpdate<com.zeroc.IceMX.ConnectionMetrics>()
        {
            @Override
            public void update(com.zeroc.IceMX.ConnectionMetrics v)
            {
                v.receivedBytes += _receivedBytes;
            }
        };

    private int _sentBytes;
    private int _receivedBytes;
}
