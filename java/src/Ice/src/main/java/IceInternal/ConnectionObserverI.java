// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class ConnectionObserverI
    extends IceMX.ObserverWithDelegate<IceMX.ConnectionMetrics, Ice.Instrumentation.ConnectionObserver>
    implements Ice.Instrumentation.ConnectionObserver
{
    @Override
    public void
    sentBytes(final int num)
    {
        _sentBytes = num;
        forEach(_sentBytesUpdate);
        if(_delegate != null)
        {
            _delegate.sentBytes(num);
        }
    }

    @Override
    public void
    receivedBytes(int num)
    {
        _receivedBytes = num;
        forEach(_receivedBytesUpdate);
        if(_delegate != null)
        {
            _delegate.receivedBytes(num);
        }
    }

    private MetricsUpdate<IceMX.ConnectionMetrics> _sentBytesUpdate = new MetricsUpdate<IceMX.ConnectionMetrics>()
        {
            @Override
            public void
            update(IceMX.ConnectionMetrics v)
            {
                v.sentBytes += _sentBytes;
            }
        };

    private MetricsUpdate<IceMX.ConnectionMetrics> _receivedBytesUpdate = new MetricsUpdate<IceMX.ConnectionMetrics>()
        {
            @Override
            public void
            update(IceMX.ConnectionMetrics v)
            {
                v.receivedBytes += _receivedBytes;
            }
        };

    private int _sentBytes;
    private int _receivedBytes;
}