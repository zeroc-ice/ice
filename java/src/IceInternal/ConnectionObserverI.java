// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class ConnectionObserverI extends IceMX.Observer<IceMX.ConnectionMetrics> 
    implements Ice.Instrumentation.ConnectionObserver
{
    public void 
    sentBytes(final int num)
    {
        _sentBytes = num;
        forEach(_sentBytesUpdate);
    }

    public void 
    receivedBytes(int num)
    {
        _receivedBytes = num;
        forEach(_receivedBytesUpdate);
    }

    private MetricsUpdate<IceMX.ConnectionMetrics> _sentBytesUpdate = new MetricsUpdate<IceMX.ConnectionMetrics>()
        {
            public void
            update(IceMX.ConnectionMetrics v)
            {
                v.sentBytes += _sentBytes;
            }
        };

    private MetricsUpdate<IceMX.ConnectionMetrics> _receivedBytesUpdate = new MetricsUpdate<IceMX.ConnectionMetrics>()
        {
            public void
            update(IceMX.ConnectionMetrics v)
            {
                v.receivedBytes += _receivedBytes;
            }
        };

    private int _sentBytes;
    private int _receivedBytes;
}