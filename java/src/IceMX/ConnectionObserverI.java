// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceMX;

public class ConnectionObserverI extends Observer<ConnectionMetrics> implements Ice.Instrumentation.ConnectionObserver
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

    private MetricsUpdate<ConnectionMetrics> _sentBytesUpdate = new MetricsUpdate<ConnectionMetrics>()
        {
            public void
            update(ConnectionMetrics v)
            {
                v.sentBytes += _sentBytes;
            }
        };

    private MetricsUpdate<ConnectionMetrics> _receivedBytesUpdate = new MetricsUpdate<ConnectionMetrics>()
        {
            public void
            update(ConnectionMetrics v)
            {
                v.receivedBytes += _receivedBytes;
            }
        };

    private int _sentBytes;
    private int _receivedBytes;
}