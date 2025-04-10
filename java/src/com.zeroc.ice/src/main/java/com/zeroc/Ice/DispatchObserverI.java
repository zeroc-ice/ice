// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import com.zeroc.Ice.IceMX.DispatchMetrics;
import com.zeroc.Ice.IceMX.ObserverWithDelegate;
import com.zeroc.Ice.Instrumentation.DispatchObserver;

/**
 * @hidden Public because it's used by IceMX (via reflection).
 */
public class DispatchObserverI extends ObserverWithDelegate<DispatchMetrics, DispatchObserver>
    implements DispatchObserver {
    @Override
    public void userException() {
        forEach(_userException);
        if (_delegate != null) {
            _delegate.userException();
        }
    }

    @Override
    public void reply(final int size) {
        forEach(
            new MetricsUpdate<DispatchMetrics>() {
                @Override
                public void update(DispatchMetrics v) {
                    v.replySize += size;
                }
            });
        if (_delegate != null) {
            _delegate.reply(size);
        }
    }

    private final MetricsUpdate<DispatchMetrics> _userException =
        new MetricsUpdate<DispatchMetrics>() {
            @Override
            public void update(DispatchMetrics v) {
                ++v.userException;
            }
        };
}
