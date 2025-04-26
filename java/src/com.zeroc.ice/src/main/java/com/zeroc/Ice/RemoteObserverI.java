// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import com.zeroc.IceMX.ObserverWithDelegate;
import com.zeroc.IceMX.RemoteMetrics;
import com.zeroc.Ice.Instrumentation.RemoteObserver;

/**
 * @hidden Public because it's used by IceMX (via reflection).
 */
public class RemoteObserverI
    extends ObserverWithDelegate<RemoteMetrics, RemoteObserver>
    implements RemoteObserver {
    @Override
    public void reply(final int size) {
        forEach(
            new MetricsUpdate<RemoteMetrics>() {
                @Override
                public void update(RemoteMetrics v) {
                    v.replySize += size;
                }
            });
        if (_delegate != null) {
            _delegate.reply(size);
        }
    }
}
