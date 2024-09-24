//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

import com.zeroc.Ice.IceMX.RemoteMetrics;
import com.zeroc.Ice.Instrumentation.RemoteObserver;

/**
 * @hidden Public because it's used by IceMX (via reflection).
 */
public class RemoteObserverI
        extends com.zeroc.Ice.IceMX.ObserverWithDelegate<RemoteMetrics, RemoteObserver>
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
