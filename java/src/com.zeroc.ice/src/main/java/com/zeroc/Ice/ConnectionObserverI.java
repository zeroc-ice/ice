// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import com.zeroc.Ice.Instrumentation.ConnectionObserver;
import com.zeroc.IceMX.ConnectionMetrics;
import com.zeroc.IceMX.ObserverWithDelegate;

/**
 * @hidden Public because it's used by IceMX (via reflection).
 */
public class ConnectionObserverI extends ObserverWithDelegate<ConnectionMetrics, ConnectionObserver>
    implements ConnectionObserver {
    @Override
    public void sentBytes(final int num) {
        _sentBytes = num;
        forEach(_sentBytesUpdate);
        if (_delegate != null) {
            _delegate.sentBytes(num);
        }
    }

    @Override
    public void receivedBytes(int num) {
        _receivedBytes = num;
        forEach(_receivedBytesUpdate);
        if (_delegate != null) {
            _delegate.receivedBytes(num);
        }
    }

    private final MetricsUpdate<ConnectionMetrics> _sentBytesUpdate =
        new MetricsUpdate<ConnectionMetrics>() {
            @Override
            public void update(ConnectionMetrics v) {
                v.sentBytes += _sentBytes;
            }
        };

    private final MetricsUpdate<ConnectionMetrics> _receivedBytesUpdate =
        new MetricsUpdate<ConnectionMetrics>() {
            @Override
            public void update(ConnectionMetrics v) {
                v.receivedBytes += _receivedBytes;
            }
        };

    private int _sentBytes;
    private int _receivedBytes;
}
